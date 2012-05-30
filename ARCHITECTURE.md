# Architecture

## Data Types

The Sky server implements some basic types of data related to paths and events.
It also has logical structures to contain this data. Below is a glossary of
these types.

1. `Object` - A thing you want to track. (e.g. A user, product)

1. `Action` - Something that an object has done. (e.g. A user signs up for a
   service)

1. `Property` - Something that describes an object. (e.g. A user's email
   address.)

1. `Event` - A single action and/or state change for an object at a given moment in
   time.


## Physical Files

### Overview

Each Sky database is a directory containing tables (which are also
directories). Tables contain the following data and control files:

1. `data` - The main data file. It is subdivided into blocks which store paths
   and events.

1. `header` - Stores object id ranges for blocks in the data file. It works as
   an index for finding paths by object id.

1. `actions` - A list of actions in the table and their associated id.

1. `properties` - A list of property keys in the table and their
   associated id.

1. `.lock` - A file used for locking the table for writing.


### Format

The following is the serialization format for all types of data within the
various database files. This is roughly in Extended Backus-Naur Form EBNF.

    # Basic Types
    TIMESTAMP = int64
    OBJECT_ID = int64
    ACTION_ID = int32

    MIN_OBJECT_ID = OBJECT_ID
    MAX_OBJECT_ID = OBJECT_ID

    # Header File
    VERSION     = uint32
    BLOCK_SIZE  = uint32
    BLOCK_COUNT = uint64
    HEADER = VERSION BLOCK_SIZE BLOCK_COUNT (MIN_OBJECT_ID MAX_OBJECT_ID)*

    # Actions File
    ACTION_COUNT = uint32
    ACTION_NAME_LENGTH = uint16
    ACTION_NAME = char*
    ACTIONS = ACTION_COUNT (ACTION_ID ACTION_NAME_LENGTH ACTION_NAME)*

    # Properties File
    PROPERTY_ID = uint16
    PROPERTY_VALUE_LENGTH = uint8
    PROPERTY_VALUE = char*
    PROPERTIES = (PROPERTY_ID PROPERTY_VALUE_LENGTH PROPERTY_VALUE)*

    # Data File
    EVENT_FLAGS = 0x01 | 0x02 | 0x03
    EVENT = EVENT_FLAGS TIMESTAMP ACTION_ID? (PROPERTY_LENGTH PROPERTIES*)?

    EVENTS_LENGTH = uint32
    PATH  = OBJECT_ID EVENTS_LENGTH EVENT*

    PATH_COUNT = uint32
    BLOCK = PATH_COUNT PATH*

The event flags are the `action` flag (`0x01`) and the `property` flag (`0x02`).
If the action flag is set then the `ACTION_ID` is expected to be present in the
event. If the property flag is set then the `PROPERTY_LENGTH` and
`PROPERTIES` elements are expected to be present in the event.


### Block Spanning

Blocks default to 64kb in size. If a path is larger than the block size then
the path must span multiple blocks. There is no facility in the path itself to
determine whether the path spans across blocks but instead the information is
derived from the `header` file.

The `header` file stores the minimum and maximum object identifier for the paths
in each block. If a block only has one path then the min and max object id will
be the same. If multiple blocks have the same object id then the path spans
blocks. The subpaths will be joined behind the scenes when traversing the path
so the query engine will not have to know about the split.


## Network Protocol

### Overview

Sky communicates with clients by sending messages over TCP in a binary format.
The message is an envelope that contains a `message version`, `message type`,
`message length` and the `message body`. The message version allows the format
to change over time while preserving backward compatibility. The message type
describes the contents of the message body. The message length states, in bytes,
how long the body will be. Below is the format for the message in EBNF:

    MSG_VERSION = uint16
    MSG_TYPE = uint32
    MSG_LENGTH = uint32
    MSG_BODY = char*
    MESSAGE = MSG_VERSION MSG_TYPE MSG_LENGTH MSG_BODY

Currently the server only supports the Event Add (`EADD`) message.


# Event Add Message (EADD)

The `EADD` message is one of the most fundamental message to Sky. It sends a
timestamped event for an object containing the action and/or any related data.

Below is the form of the body of the EADD message in EBNF:

    DB_NAME_LENGTH = uint8
    DB_NAME = char*
    OBJ_FILE_NAME_LENGTH = uint8
    OBJ_FILE_NAME_NAME = char*
    OBJECT_ID = int64
    TIMESTAMP = int64
    ACTION_NAME_LENGTH = uint16
    ACTION_NAME = char*
    DATA_COUNT = uint32
    DATA_KEY_LENGTH = uint16
    DATA_KEY = char*
    DATA_VALUE_LENGTH = uint8
    DATA_VALUE = char*
    
    MSG_BODY = OBJECT_ID TIMESTAMP ACTION_NAME_LENGTH ACTION_NAME DATA_COUNT \
               (DATA_KEY_LENGTH DATA_KEY DATA_VALUE_LENGTH DATA_VALUE)*

