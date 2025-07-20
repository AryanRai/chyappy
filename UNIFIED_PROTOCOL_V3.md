# Chyappy Unified Protocol v3.0

## Overview

The Chyappy Unified Protocol v3.0 integrates the original Chyappy binary protocol with WebSocket JSON messaging for seamless communication between hardware sensors, physics simulations, trading systems, and user interfaces.

## Protocol Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Hardware      │    │  Stream Handler  │    │    AriesUI      │
│   Sensors       │◄──►│     v3.0         │◄──►│   (WebSocket)   │
│  (Chyappy)      │    │   (WebSocket)    │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                              ▲
                              │
                       ┌──────▼──────┐
                       │   StarSim   │
                       │  (Physics)  │
                       │ (WebSocket) │
                       └─────────────┘
```

## Message Types

### 1. Negotiation Messages
Used for initial stream discovery and registration.

```json
{
  "type": "negotiation",
  "status": "active",
  "data": {
    "stream_id_1": {
      "stream_id": "spring_mass_system_position",
      "name": "StarSim Position",
      "datatype": "float",
      "unit": "m",
      "value": 0.5,
      "status": "active",
      "timestamp": "2025-07-19T20:25:35.123Z",
      "simulation_id": "spring_mass_system"
    },
    "stream_id_2": {
      "stream_id": "sensor_module_temperature",
      "name": "Chamber Temperature",
      "datatype": "float",
      "unit": "°C",
      "value": 23.5,
      "status": "active",
      "timestamp": "2025-07-19T20:25:35.124Z",
      "sensor_type": "T",
      "sensor_id": 1,
      "sequence_number": 42
    }
  },
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

### 2. Physics Simulation Messages
Used for StarSim physics engine communication.

#### Registration
```json
{
  "type": "physics_simulation",
  "action": "register",
  "simulation_id": "spring_mass_system",
  "config": {
    "name": "ParsecCore Simulation",
    "solver": "RK4",
    "dt": 0.001
  },
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

#### Stream Registration
```json
{
  "type": "physics_simulation",
  "action": "register_stream",
  "simulation_id": "spring_mass_system",
  "stream_id": "position",
  "stream_data": {
    "name": "Position",
    "datatype": "float",
    "unit": "m",
    "value": 0.0,
    "status": "active",
    "timestamp": "2025-07-19T20:25:35.123Z"
  },
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

#### Data Updates
```json
{
  "type": "physics_simulation",
  "action": "update",
  "simulation_id": "spring_mass_system",
  "stream_id": "position",
  "data": {
    "value": 0.999,
    "timestamp": "2025-07-19T20:25:35.123Z"
  },
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

#### Control Commands
```json
{
  "type": "physics_simulation",
  "action": "control",
  "simulation_id": "spring_mass_system",
  "command": "start",
  "params": {
    "speed": 1.0
  },
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

### 3. Trading Stream Messages
Used for financial data and trading systems.

```json
{
  "type": "trading_stream",
  "symbol": "SPY",
  "streams": {
    "price": {
      "stream_id": "SPY_price",
      "name": "SPY Price",
      "datatype": "float",
      "unit": "USD",
      "value": 445.67,
      "status": "active",
      "timestamp": "2025-07-19T20:25:35.123Z"
    }
  },
  "market_data": {
    "bid": 445.65,
    "ask": 445.69,
    "volume": 1234567
  },
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

### 4. Ping/Pong Messages
Used for connection monitoring and latency measurement.

#### Ping
```json
{
  "type": "ping",
  "timestamp": 1752920735.4625802,
  "target": "sh",
  "status": "active",
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

#### Pong
```json
{
  "type": "pong",
  "timestamp": 1752920735.4625802,
  "target": "ui",
  "server_time": 1752920735.4626,
  "status": "active",
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

## Data Types

### Supported Data Types
- `float`: Floating-point numbers
- `int`: Integer numbers
- `string`: Text data
- `bool`: Boolean values
- `vector2`: 2D vectors [x, y]
- `vector3`: 3D vectors [x, y, z]

### Chyappy Protocol Mapping
The protocol maintains compatibility with the original Chyappy binary format:

| Chyappy Type | JSON Type | Payload Type |
|--------------|-----------|--------------|
| String       | string    | 0x01         |
| Float        | float     | 0x02         |
| Int16        | int       | 0x03         |
| Int32        | int       | 0x04         |

## Stream ID Format

### Physics Simulations
Format: `{simulation_id}_{stream_id}`
Example: `spring_mass_system_position`

### Hardware Sensors
Format: `{module_id}_{sensor_type}_{sensor_id}`
Example: `sensor_hub_T_1` (Temperature sensor #1)

### Trading Streams
Format: `{symbol}_{data_type}`
Example: `SPY_price`

## Connection Flow

### 1. Initial Connection
1. Client connects to Stream Handler WebSocket (port 3000)
2. Stream Handler sends initial ping
3. Client responds with pong
4. Client sends negotiation request
5. Stream Handler responds with available streams

### 2. Stream Subscription
1. Client subscribes to specific streams
2. Stream Handler begins sending updates
3. Updates are sent in real-time as data changes

### 3. Physics Simulation Integration
1. StarSim connects to Stream Handler
2. Registers simulation and streams
3. Sends real-time physics data
4. Receives control commands from UI

## Error Handling

### Error Message Format
```json
{
  "type": "error",
  "error": "Simulation spring_mass_system not found",
  "msg-sent-timestamp": "2025-07-19 20:25:35"
}
```

### Common Error Codes
- `simulation_not_found`: Physics simulation doesn't exist
- `stream_not_found`: Requested stream doesn't exist
- `invalid_command`: Unrecognized control command
- `connection_failed`: WebSocket connection error

## Implementation Examples

### AriesUI Connection
```typescript
import { commsClient } from '@/lib/comms-stream-client'

// Connect to stream handler
await commsClient.connect()

// Subscribe to physics stream
commsClient.subscribeToStream('spring_mass_system_position', (value, metadata) => {
  console.log('Position:', value, metadata.unit)
})
```

### StarSim C++ Integration
```cpp
#include "parsec/InputManager.h"

// Create input manager
parsec::InputManager input_manager("spring_mass_system");
input_manager.initialize("ws://localhost:3000");

// Register stream
input_manager.registerStream("position", "Position", "float", "m");

// Send updates
input_manager.updateStreamValue("position", 0.999);
```

### Python Hardware Module
```python
import json
import websocket

# Connect to stream handler
ws = websocket.WebSocket()
ws.connect("ws://localhost:3000")

# Send sensor data
message = {
    "type": "negotiation",
    "status": "active",
    "data": {
        "sensor_T_1": {
            "stream_id": "sensor_T_1",
            "name": "Temperature Sensor 1",
            "datatype": "float",
            "unit": "°C",
            "value": 23.5,
            "status": "active",
            "timestamp": datetime.now().isoformat(),
            "sensor_type": "T",
            "sensor_id": 1
        }
    },
    "msg-sent-timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
}

ws.send(json.dumps(message))
```

## Configuration

### Stream Handler Configuration
- **Port**: 3000 (WebSocket)
- **Max Payload**: 16MB
- **Idle Timeout**: 60 seconds
- **Compression**: Enabled

### Connection Parameters
- **Reconnection Attempts**: 10
- **Reconnection Delay**: Exponential backoff (1s, 2s, 4s, ...)
- **Ping Interval**: 100ms
- **Debug Level**: Configurable (0-2)

## Migration Guide

### From Chyappy v1.2 to Unified v3.0

1. **Message Format**: Binary messages are now JSON
2. **Stream IDs**: Use new unified format
3. **Data Types**: Extended type support
4. **Connection**: WebSocket instead of serial/TCP
5. **Error Handling**: Structured error messages

### Backward Compatibility
- Original Chyappy sensor types are preserved
- Sensor IDs and sequence numbers maintained
- Payload type mapping for existing systems

## Testing

### Connection Test
```bash
# Test WebSocket connection
wscat -c ws://localhost:3000

# Send test message
{"type":"negotiation","status":"active","data":{},"msg-sent-timestamp":"2025-07-19 20:25:35"}
```

### Stream Handler Status
```bash
# Check status endpoint
curl http://localhost:3000/status
```

## Troubleshooting

### Common Issues

1. **Connection Refused**
   - Ensure Stream Handler is running on port 3000
   - Check firewall settings

2. **No Stream Data**
   - Verify stream registration
   - Check stream ID format
   - Confirm data source is active

3. **High Latency**
   - Monitor ping/pong messages
   - Check network conditions
   - Verify message size limits

### Debug Mode
Enable verbose logging in Stream Handler:
```python
VERBOSE_LEVEL = 2  # Maximum debug output
```

## Future Enhancements

- Binary WebSocket support for high-frequency data
- Stream compression for large datasets
- Authentication and authorization
- Stream filtering and aggregation
- Historical data storage integration