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

Each Sky database is a directory containing object files (which are also
directories). Object files contain the following data and control files:

1. `data` - The main data file. It is subdivided into blocks which store paths
   and events.

1. `header` - Stores object id ranges for blocks in the data file. It works as
   an index for finding paths by object id.

1. `actions` - A list of actions in the object file and their associated id.

1. `properties` - A list of property keys in the object file and their
   associated id.

1. `.lock` - A file used for locking the object file for writing.


### Format

The following is the serialization format for all types of data within the
various database files. This is roughly in Extended Backus-Naur Form EBNF.

    # Basic Types
    TIMESTAMP = int64
    ACTION_ID = int64
    OBJECT_ID = int64

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
    PROPERTY_COUNT = uint32
    PROPERTY_NAME_LENGTH = uint16
    PROPERTY_NAME = char*
    PROPERTIES = PROPERTY_COUNT (PROPERTY_ID PROPERTY_NAME_LENGTH PROPERTY_NAME)*

    # Data File
    EVENT_CODE = 0x01 | 0x11 | 0x21 | 0x31
    EVENT = EVENT_CODE TIMESTAMP ACTION_ID? (PROPERTY_LENGTH PROPERTY_CHANGE*)?

    EVENTS_LENGTH = uint32
    PATH  = 0x02 OBJECT_ID EVENTS_LENGTH EVENT*

    PATHS_LENGTH = uint32
    BLOCK = 0x03 PATHS_LENGTH PATH*

The code byte is the first byte in an event, path or block. It is split in two
halves: the first 4 bits are options and the second 4 bits represent the code.
The code identifies the type of data (EVENT=1, PATH=2, BLOCK=3). The options
are only used by the event.

The event options have two flags: `ACTION FLAG=0001XXXX`,
`PROPERTY FLAG=00001XXXX`. If the action flag is set then the `ACTION ID` is
expected to be present in the event. If the property flag is set then the
`PROPERTY LENGTH` and `PROPERTY CHANGE` elements are expected to be present in
the event.


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
