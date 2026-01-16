/* Copyright (C) 2024 - RFB Protocol Extensions for Latency
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

// -=- RFBProtocolExt.h - RFB Protocol Extensions for Latency Metrics

#ifndef __RFB_RFBPROTOCOLEXT_H__
#define __RFB_RFBPROTOCOLEXT_H__

#include <rdr/types.h>

namespace rfb {

  // Server-to-Client message type for latency statistics
  // Message type: 253 (custom message type in RFB protocol reserved range)
  struct LatencyStatsMessage {
    rdr::U8 type;                    // Message type (253)
    rdr::U8 padding;                 // Padding for alignment
    rdr::U16 messageLength;          // Total message length in bytes
    rdr::U32 clickLatencyMs;         // Last click latency in milliseconds
    rdr::U32 displayLatencyMs;       // Last display latency in milliseconds
    rdr::U32 avgClickLatencyMs;      // Average click latency in milliseconds
    rdr::U32 avgDisplayLatencyMs;    // Average display latency in milliseconds
    rdr::U32 minClickLatencyMs;      // Minimum click latency in milliseconds
    rdr::U32 maxClickLatencyMs;      // Maximum click latency in milliseconds
    rdr::U32 minDisplayLatencyMs;    // Minimum display latency in milliseconds
    rdr::U32 maxDisplayLatencyMs;    // Maximum display latency in milliseconds
    rdr::U32 clickCount;             // Number of click samples
    rdr::U32 displayCount;           // Number of display samples
    rdr::U64 timestamp;              // Unix timestamp in milliseconds
    
    LatencyStatsMessage() : type(253), padding(0), messageLength(sizeof(LatencyStatsMessage)),
                           clickLatencyMs(0), displayLatencyMs(0),
                           avgClickLatencyMs(0), avgDisplayLatencyMs(0),
                           minClickLatencyMs(0), maxClickLatencyMs(0),
                           minDisplayLatencyMs(0), maxDisplayLatencyMs(0),
                           clickCount(0), displayCount(0), timestamp(0) {}
  };

  // Protocol extension message types
  enum ProtocolExtMessageTypes {
    LATENCY_STATS_MESSAGE = 253  // Send latency statistics to client
  };

  // Server-to-Client message type: Request Latency Stats (optional polling)
  struct RequestLatencyStatsMessage {
    rdr::U8 type;              // Message type (254)
    rdr::U8 padding[3];        // Padding for alignment
    
    RequestLatencyStatsMessage() : type(254) { 
      padding[0] = padding[1] = padding[2] = 0;
    }
  };

} // namespace rfb

#endif // __RFB_RFBPROTOCOLEXT_H__
