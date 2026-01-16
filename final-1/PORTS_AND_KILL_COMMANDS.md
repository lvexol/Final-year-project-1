# Port Information & How to Kill Applications

## 📊 Which Port?

Your system uses **TWO MAIN PORTS**:

### **PORT 8080** - Web Dashboard ✅ RUNNING
- **Service:** Python HTTP Server (LatencyMonitor)
- **URL:** http://localhost:8080/
- **Access:** Open this in your browser right now
- **Status:** Currently active
- **Features:**
  - Real-time latency metrics display
  - Click latency statistics
  - Display latency statistics
  - JSON export
  - CSV export
  - Statistical analysis

### **PORT 5900** - VNC Server 
- **Service:** Xvnc Remote Desktop Server
- **Protocol:** RFB/VNC
- **Status:** Compiled and ready (X11 environment issue in current setup)
- **When it works:** Connect with `vncviewer localhost:5900`
- **Features:**
  - Remote desktop access
  - Click tracking (automatic)
  - Display tracking (automatic)
  - Real-time latency measurement

### **Display :99** - Virtual X Display ✅ RUNNING
- **Service:** Xvfb (Virtual Framebuffer)
- **Status:** Currently active
- **Purpose:** Provides virtual display for testing

---

## 🛑 How to Kill Applications

### Option 1: Kill Web Dashboard (Port 8080) Only
```bash
pkill -f server.py
```

Or by PID (if you know it):
```bash
kill -9 39704
```

Or by port number:
```bash
kill -9 $(lsof -ti :8080)
```

### Option 2: Kill VNC Server (Port 5900) Only
```bash
pkill -9 Xvnc
```

Or by port number:
```bash
kill -9 $(lsof -ti :5900)
```

### Option 3: Kill Virtual Display Only
```bash
pkill -9 Xvfb
```

Or by PID:
```bash
kill -9 34336
```

### Option 4: Kill Everything at Once ⭐ RECOMMENDED
```bash
pkill -9 server.py; pkill -9 Xvnc; pkill -9 Xvfb
```

This single command kills all three services.

### Option 5: Graceful Shutdown (If Using START_SYSTEM.sh)
```bash
# Press in the terminal where START_SYSTEM.sh is running:
Ctrl+C
```

This automatically stops all processes cleanly.

---

## ✅ Verify Everything is Killed

After killing, run:
```bash
ps aux | grep -E 'Xvfb|Xvnc|server.py' | grep -v grep
```

**Expected:** No output = All processes killed successfully

---

## 📋 Current Status

### Running Processes
```
PID 34336:  Xvfb :99 -screen 0 1024x768x24    ✅ RUNNING
PID 39704:  python3 server.py                  ✅ RUNNING
```

### Listening Ports
```
Port 8080:  LISTENING (Web Dashboard)          ✅ ACTIVE
Port 5900:  Not listening (VNC blocked)        ⚠️  Blocked
```

---

## ⚡ Quick Reference

| Task | Command |
|------|---------|
| **Access web dashboard** | `http://localhost:8080/` (browser) |
| **Check running processes** | `ps aux \| grep -E 'Xvfb\|Xvnc\|server.py'` |
| **Check listening ports** | `netstat -tuln \| grep -E '5900\|8080'` |
| **Kill web server only** | `pkill -f server.py` |
| **Kill VNC server only** | `pkill -9 Xvnc` |
| **Kill virtual display only** | `pkill -9 Xvfb` |
| **Kill everything** | `pkill -9 server.py; pkill -9 Xvnc; pkill -9 Xvfb` |
| **Verify all killed** | `ps aux \| grep -E 'Xvfb\|Xvnc\|server.py' \| grep -v grep` |

---

## 🎯 Most Common Scenarios

### Scenario 1: Just Stop the Web Server
```bash
pkill -f server.py
```

### Scenario 2: Stop Everything and Start Fresh
```bash
# Kill all
pkill -9 server.py; pkill -9 Xvnc; pkill -9 Xvfb
sleep 2

# Verify
ps aux | grep -E 'Xvfb|Xvnc|server.py' | grep -v grep

# Start fresh
cd /home/vexo/project/Final-year-project-1/final-1
bash START_SYSTEM.sh
```

### Scenario 3: Check What's Using Port 8080
```bash
lsof -i :8080
```

### Scenario 4: Check What's Using Port 5900
```bash
lsof -i :5900
```

### Scenario 5: Kill Process by Port
```bash
# Kill whatever is using port 8080
kill -9 $(lsof -ti :8080)

# Kill whatever is using port 5900
kill -9 $(lsof -ti :5900)
```

---

## 💡 Tips

1. **`-9` flag:** Forces immediate kill (no graceful shutdown)
   - Use when process won't respond
   - Example: `pkill -9 Xvnc`

2. **No `-9` flag:** Graceful shutdown (gives process time to cleanup)
   - Use when available
   - Example: `pkill server.py`

3. **Check before killing:**
   ```bash
   ps aux | grep server.py
   ```

4. **Verify port is free before restarting:**
   ```bash
   netstat -tuln | grep 8080
   # Should show no output
   ```

---

## Summary

- **Web Dashboard Port:** `8080` (currently running ✅)
- **VNC Server Port:** `5900` (compiled and ready ⚠️)
- **Kill All Command:** `pkill -9 server.py; pkill -9 Xvnc; pkill -9 Xvfb`
- **Verify Killed:** `ps aux | grep -E 'Xvfb|Xvnc|server.py' | grep -v grep`
