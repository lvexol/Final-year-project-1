#!/usr/bin/python
import scapy.all as scapy
import datetime
import logging
import argparse

time = datetime.datetime.today()

logging.basicConfig(level=logging.INFO, format="%(asctime)s [%(levelname)s] %(message)s")

parser = argparse.ArgumentParser(
    prog='BWIDS',
    description='BWIDS is a simple WiFi network monitoring system.',
    epilog='Example usage: sudo python3 bwids.py --iface="wlp2s0" --router_mac="AA:AA:AA:AA:AA:AA"')

parser.add_argument('--iface', type=str, dest='iface', help="Specify interface", required=True)
parser.add_argument('--router_mac', type=str, dest='router_mac',
                    help='Specify the MAC of your router', required=True)
args = parser.parse_args()

iface = args.iface
router_mac = args.router_mac
db_list = []
client_list = []


def get_mac(ip):
    '''We send an ARP request to the specified IP.
    If it is answered by another MAC as declared in the earlier ARP response it means that,
    the array was poisoned by the attacker - MiTM Attack.'''
    p = scapy.Ether(dst='ff:ff:ff:ff:ff:ff') / scapy.ARP(pdst=ip)
    result = scapy.srp(p, timeout=3, verbose=False)[0]
    return result[0][1].hwsrc


def filter(pkt):
    global router_mac
    global client_list

    ''' When a deauth frame from the target AP appears, a message will appear.
        Many messages signify an authentication revocation attack,
        which often only precedes more dangerous attacks.'''
    if pkt.haslayer(scapy.Dot11Deauth):
        mac = str(pkt.addr2).upper()
        if mac != router_mac and router_mac == str(pkt.addr1).upper():
            log = "Deauthentication attack detected against MAC address: " + str(pkt.addr2).upper()
            logging.warning(log)
            msg = "Deauth frame."
            log_to_file(str(pkt.addr1), str(pkt.addr2), msg, "INFO")
            if str(pkt.addr2).upper() in client_list:
                client_list.remove(str(pkt.addr2).upper())


    ''' Based on the fact that the signal strength of our network is contained in a given range,
        at the time of occurrence of ap with a much better signal a message will be thrown out,
        indicating a high probability of the appearance of fake AP'''
    if pkt.haslayer(scapy.Dot11ProbeResp):
        global db_list
        try:
            dbm_signal = pkt.dBm_AntSignal
        except:
            dbm_signal = "N/A"
        if router_mac == str(pkt.addr2).upper():
            if dbm_signal != "N/A":
                if db_list:
                    if 0 < len(db_list) < 10:
                        db_list.append(dbm_signal)
                    else:
                        average = sum(db_list) / len(db_list)
                        if average - 20 < dbm_signal < average + 20:
                            db_list.append(dbm_signal)
                        else:
                            log = "Probe response detected on MAC address: " + str(pkt.addr2).upper()
                            + ", of the value: " + str(dbm_signal) + " dBm, to the router: " + str(pkt.info)
                            logging.warning(log)
                            msg = "Fake AP found."
                            log_to_file(str(pkt.addr1), str(pkt.addr2), msg, "ALERT")
                else:
                    db_list.append(dbm_signal)
            log = " [ " + str(time) + " ] Probe response detected on MAC address: "
            + str(pkt.addr2).upper() + ", of the value: " + str(dbm_signal)
            + " dBm, to the router: " + str(pkt.info)
            msg = "Probe Response received"
            log_to_file(str(pkt.addr1), str(pkt.addr2), msg, "INFO")

    '''adds newly connected clients to the list'''
    if pkt.haslayer(scapy.Dot11AssoResp):
        if router_mac == str(pkt.addr2).upper():
            if str(pkt.addr1).upper() in client_list:
                pass
            else:
                client_list.append(str(pkt.addr1).upper())
                log = "New client connected: " + str(pkt.addr1).upper()
                logging.info(log)
                msg = "Client connected."
                log_to_file(str(pkt.addr1), str(pkt.addr2), msg, "INFO")

    '''Detects MiTM attacks by comparing physical addresses.
       Works only on the network we are connected to.'''
    if pkt.haslayer(scapy.ARP):
        if pkt[scapy.ARP].op == 1:
            log = "ARP Request Reply detected on MAC: " + pkt[scapy.ARP].hwsrc
            logging.warning(log)
        if pkt[scapy.ARP].op == 2:
            try:
                real_mac = get_mac(pkt[scapy.ARP].psrc)
                response_mac = pkt[scapy.ARP].hwsrc
                if real_mac != response_mac:
                    log = f"Man in The Middle detected on MAC: {real_mac.upper()},"
                    f"MAC of the attacker: {response_mac.upper()} !!"
                    logging.warning(log)
                    msg = "MiTM attack"
                    log_to_file(str(real_mac), str(response_mac), msg, "ALERT")
            except IndexError:
                pass


def log_to_file(source_mac, dest_mac, alert, type):
    '''A function that saves logs to a file, which can be integrated with a data monitoring and
    visualization system (e.g. Splunk)'''
    message = "type= " + type + ", source_mac= " + source_mac + ", dest_mac= " + dest_mac
    + ", alert= " + alert + "\n"
    with open("logfile.log", "a") as logfile:
        logfile.write(message)


print("basic_wireless_intrusion_detection_system running...")

# Constantly listening for traffic
capture = scapy.sniff(iface=iface, prn=filter)
# capture.summary()
