# Hephabus UDP implementation

## Basic idea

All nodes are connected via Ethernet and optionally supplied with power via PoE mode B.
Nodes open an UDP port and listen to incoming messages.
Each node has a hardcoded list of potentially interested subscribers and sends updates to them on state changes.

## Messages

Each message has the following format (in bytes):

1 byte command type (update, set, modify)
2 byte (uint16) endpoint number
8 byte (int64) payload

node ids are encoded in IP.

### commands

0. `update`: publish state changes
1. `set`: set new value (output only)
2. `modify`: change value relative to current

### reserved endpoint numbers

0. node update state (regular ping)

