/**
 * LatencyMonitor.js - Frontend component for displaying latency metrics
 * 
 * This module provides real-time monitoring of click and display latency
 * and displays the metrics in the KasmVNC web interface.
 */

class LatencyMonitor {
  constructor(options = {}) {
    this.options = {
      updateInterval: options.updateInterval || 1000,  // Update every 1 second
      maxHistorySize: options.maxHistorySize || 100,    // Keep last 100 measurements
      displayElement: options.displayElement || null,   // Target DOM element
      enabled: options.enabled !== false,               // Enabled by default
      ...options
    };

    this.clickLatencies = [];
    this.displayLatencies = [];
    this.stats = {
      lastClickLatency: 0,
      lastDisplayLatency: 0,
      avgClickLatency: 0,
      avgDisplayLatency: 0,
      minClickLatency: 0,
      maxClickLatency: 0,
      minDisplayLatency: 0,
      maxDisplayLatency: 0,
      clickCount: 0,
      displayCount: 0
    };

    this.initialized = false;
    this.updateTimer = null;
  }

  /**
   * Initialize the latency monitor
   */
  init() {
    if (this.initialized) return;
    
    if (!this.options.displayElement) {
      this.createDefaultUI();
    }
    
    this.initialized = true;
    
    if (this.options.enabled) {
      this.startMonitoring();
    }

    console.log('LatencyMonitor initialized');
  }

  /**
   * Create default UI if no display element provided
   */
  createDefaultUI() {
    const container = document.createElement('div');
    container.id = 'latency-monitor-container';
    container.style.cssText = `
      position: fixed;
      bottom: 10px;
      right: 10px;
      width: 300px;
      background: rgba(0, 0, 0, 0.9);
      color: #00ff00;
      padding: 15px;
      border-radius: 8px;
      font-family: 'Courier New', monospace;
      font-size: 12px;
      z-index: 9999;
      box-shadow: 0 0 10px rgba(0, 255, 0, 0.3);
      border: 1px solid #00ff00;
    `;

    const header = document.createElement('div');
    header.style.cssText = `
      font-weight: bold;
      margin-bottom: 10px;
      border-bottom: 1px solid #00ff00;
      padding-bottom: 5px;
    `;
    header.textContent = 'Latency Monitor';

    const content = document.createElement('div');
    content.id = 'latency-monitor-content';
    content.style.cssText = `
      line-height: 1.6;
    `;

    container.appendChild(header);
    container.appendChild(content);
    document.body.appendChild(container);

    this.options.displayElement = document.getElementById('latency-monitor-content');
  }

  /**
   * Start monitoring and update UI
   */
  startMonitoring() {
    if (this.updateTimer) clearInterval(this.updateTimer);
    
    this.updateTimer = setInterval(() => {
      this.updateDisplay();
    }, this.options.updateInterval);
  }

  /**
   * Stop monitoring
   */
  stopMonitoring() {
    if (this.updateTimer) {
      clearInterval(this.updateTimer);
      this.updateTimer = null;
    }
  }

  /**
   * Record a click latency measurement
   * @param {number} latencyMs - Latency in milliseconds
   */
  recordClickLatency(latencyMs) {
    this.clickLatencies.push(latencyMs);
    if (this.clickLatencies.length > this.options.maxHistorySize) {
      this.clickLatencies.shift();
    }
    
    this.updateStats();
  }

  /**
   * Record a display latency measurement
   * @param {number} latencyMs - Latency in milliseconds
   */
  recordDisplayLatency(latencyMs) {
    this.displayLatencies.push(latencyMs);
    if (this.displayLatencies.length > this.options.maxHistorySize) {
      this.displayLatencies.shift();
    }
    
    this.updateStats();
  }

  /**
   * Update statistics from recorded measurements
   */
  updateStats() {
    if (this.clickLatencies.length > 0) {
      this.stats.lastClickLatency = this.clickLatencies[this.clickLatencies.length - 1];
      this.stats.minClickLatency = Math.min(...this.clickLatencies);
      this.stats.maxClickLatency = Math.max(...this.clickLatencies);
      this.stats.avgClickLatency = this.clickLatencies.reduce((a, b) => a + b, 0) / this.clickLatencies.length;
      this.stats.clickCount = this.clickLatencies.length;
    }

    if (this.displayLatencies.length > 0) {
      this.stats.lastDisplayLatency = this.displayLatencies[this.displayLatencies.length - 1];
      this.stats.minDisplayLatency = Math.min(...this.displayLatencies);
      this.stats.maxDisplayLatency = Math.max(...this.displayLatencies);
      this.stats.avgDisplayLatency = this.displayLatencies.reduce((a, b) => a + b, 0) / this.displayLatencies.length;
      this.stats.displayCount = this.displayLatencies.length;
    }
  }

  /**
   * Update the display with current statistics
   */
  updateDisplay() {
    if (!this.options.displayElement) return;

    const statusColor = (latency) => {
      if (latency < 50) return '#00ff00';      // Green - good
      if (latency < 100) return '#ffff00';     // Yellow - acceptable
      return '#ff0000';                         // Red - poor
    };

    const html = `
      <div style="margin-bottom: 8px;">
        <div><strong>Click Latency:</strong></div>
        <div style="padding-left: 10px;">
          Last: <span style="color: ${statusColor(this.stats.lastClickLatency)}">${this.stats.lastClickLatency.toFixed(1)}ms</span><br/>
          Avg: <span style="color: ${statusColor(this.stats.avgClickLatency)}">${this.stats.avgClickLatency.toFixed(1)}ms</span><br/>
          Min/Max: ${this.stats.minClickLatency.toFixed(1)}ms / ${this.stats.maxClickLatency.toFixed(1)}ms<br/>
          Samples: ${this.stats.clickCount}
        </div>
      </div>
      <div>
        <div><strong>Display Latency:</strong></div>
        <div style="padding-left: 10px;">
          Last: <span style="color: ${statusColor(this.stats.lastDisplayLatency)}">${this.stats.lastDisplayLatency.toFixed(1)}ms</span><br/>
          Avg: <span style="color: ${statusColor(this.stats.avgDisplayLatency)}">${this.stats.avgDisplayLatency.toFixed(1)}ms</span><br/>
          Min/Max: ${this.stats.minDisplayLatency.toFixed(1)}ms / ${this.stats.maxDisplayLatency.toFixed(1)}ms<br/>
          Samples: ${this.stats.displayCount}
        </div>
      </div>
    `;

    this.options.displayElement.innerHTML = html;
  }

  /**
   * Export statistics as JSON
   */
  exportJSON() {
    return JSON.stringify({
      timestamp: new Date().toISOString(),
      statistics: this.stats,
      history: {
        clickLatencies: this.clickLatencies,
        displayLatencies: this.displayLatencies
      }
    }, null, 2);
  }

  /**
   * Export statistics as CSV
   */
  exportCSV() {
    let csv = 'Type,Latency (ms)\n';
    
    this.clickLatencies.forEach(latency => {
      csv += `Click,${latency}\n`;
    });
    
    this.displayLatencies.forEach(latency => {
      csv += `Display,${latency}\n`;
    });
    
    return csv;
  }

  /**
   * Get current statistics
   */
  getStatistics() {
    return { ...this.stats };
  }

  /**
   * Clear all recorded measurements
   */
  clear() {
    this.clickLatencies = [];
    this.displayLatencies = [];
    this.stats = {
      lastClickLatency: 0,
      lastDisplayLatency: 0,
      avgClickLatency: 0,
      avgDisplayLatency: 0,
      minClickLatency: 0,
      maxClickLatency: 0,
      minDisplayLatency: 0,
      maxDisplayLatency: 0,
      clickCount: 0,
      displayCount: 0
    };
  }

  /**
   * Enable/disable monitoring
   */
  setEnabled(enabled) {
    this.options.enabled = enabled;
    if (enabled) {
      this.startMonitoring();
    } else {
      this.stopMonitoring();
    }
  }

  /**
   * Destroy the monitor
   */
  destroy() {
    this.stopMonitoring();
    const container = document.getElementById('latency-monitor-container');
    if (container) {
      container.remove();
    }
    this.initialized = false;
  }
}

// Export for use in modules
if (typeof module !== 'undefined' && module.exports) {
  module.exports = LatencyMonitor;
}
