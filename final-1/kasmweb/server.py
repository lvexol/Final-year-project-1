#!/usr/bin/env python3
"""
Simple HTTP Server for KasmVNC Latency Dashboard
Serves the HTML/CSS/JS dashboard
"""

import http.server
import socketserver
import os
import sys
import json
from pathlib import Path
from datetime import datetime
import random

PORT = 8080

class LatencyDashboardHandler(http.server.SimpleHTTPRequestHandler):
    
    def do_GET(self):
        """Handle GET requests"""
        
        # API endpoints
        if self.path == '/api/metrics':
            return self.serve_metrics_api()
        elif self.path == '/api/status':
            return self.serve_status_api()
        elif self.path.endswith('.html'):
            self.path = '/latency-dashboard.html'
        
        # Serve static files
        return super().do_GET()
    
    def serve_metrics_api(self):
        """Serve simulated latency metrics"""
        # This will be populated with real data once server runs
        metrics = {
            "status": "VNC server starting",
            "click_latency": {
                "last": 0,
                "average": 0,
                "min": 0,
                "max": 0,
                "measurements": 0
            },
            "display_latency": {
                "last": 0,
                "average": 0,
                "min": 0,
                "max": 0,
                "measurements": 0
            },
            "timestamp": datetime.now().isoformat()
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(metrics).encode())
    
    def serve_status_api(self):
        """Serve server status"""
        status = {
            "server": "KasmVNC Latency Tracking System",
            "version": "1.4.0",
            "dashboard_url": f"http://localhost:{PORT}/",
            "rfb_port": 5900,
            "status": "Dashboard Active - Awaiting VNC Connection",
            "uptime": "starting",
            "features": [
                "Click Latency Measurement",
                "Display Latency Measurement",
                "Real-time Metrics",
                "JSON Export",
                "CSV Export"
            ]
        }
        
        self.send_response(200)
        self.send_header('Content-type', 'application/json')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(json.dumps(status, indent=2).encode())

    def end_headers(self):
        """Add CORS headers"""
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        super().end_headers()

if __name__ == '__main__':
    # Change to web directory
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    
    # Start server
    with socketserver.TCPServer(("", PORT), LatencyDashboardHandler) as httpd:
        print("╔═══════════════════════════════════════════════════════════╗")
        print("║   KasmVNC Latency Dashboard - Web Server                 ║")
        print("╚═══════════════════════════════════════════════════════════╝")
        print()
        print(f"🌐 Dashboard URL: http://localhost:{PORT}/")
        print(f"📊 Metrics API: http://localhost:{PORT}/api/metrics")
        print(f"📋 Status API: http://localhost:{PORT}/api/status")
        print()
        print(f"Server listening on port {PORT}...")
        print("Press Ctrl+C to stop")
        print()
        
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n✅ Server stopped")
            sys.exit(0)
