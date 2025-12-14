# Documentation

I built a distributed key–value store using C++ socket programming. The system runs on **three or more server nodes** and behaves like a **single logical database**. Each server maintains a local copy of the data, and all write operations are **replicated** to keep the cluster in sync.

---

## Basic Functionality

Each server maintains a local `std::unordered_map<std::string, std::string>`.

When any server receives a **SET** or **DELETE** command from a client, it:

1. Updates its local map
2. Sends a **REPLICATE** message to all other servers
3. Other servers apply the same update

Read operations (**GET**) are handled locally.

---

## Client Commands

A client can connect to any server and issue the following commands:

```
SET <key> <value>
GET <key>
DELETE <key>
```

### Examples

```
SET a 10
GET a
DELETE a
```

## Multithreading

- Each client connection is handled in a **separate thread**
- A `std::mutex` protects access to the shared key–value store
- This allows multiple clients to interact with the server concurrently without data races

---

## Failure Handling

Since each server maintains its own local copy of the data, if one server goes down, clients can still connect to other servers. Replication ensures data remains available as long as at least one server is running. (This model does not implement leader election.)

---

## How to Run

### Compile

```bash
g++ server5000.cpp -o server5000 -pthread
g++ server5001.cpp -o server5001 -pthread
g++ server5002.cpp -o server5002 -pthread
```

### Start Servers (in separate terminals)

```bash
./server5000 5000
./server5001 5001
./server5002 5002
```

### Start Client

```bash
nc localhost <port_no>
```
---
## Example
<img width="2486" height="856" alt="image" src="https://github.com/user-attachments/assets/ffbadf34-04aa-4517-903d-16f4daf1371d" />


## What I Learnt

I started off with learning the basics of networking, the TCP/IP protocol stack, differences between TCP and UDP, etc. I implemented TCP sockets in C++, handling socket creation, binding, listening, accepting connections, and managing client–server communication.

I then went on to learn how to handle multiple client connections using std::thread, and shared state is protected using std::mutex. This helped me understand synchronization, and thread safety.

Using these concepts, I implemented a key–val store server that supports basic database operations such as SET, GET, and DELETE, with proper command parsing and buffered input handling to deal with partial TCP reads.

Finally, I extended the system into a replicated distributed database. I added a simple replication protocol where all write operations are propagated to peer servers using replication commands. This way, the local copies of other servers are updated as well.

---
PS: I tried to focus on understanding networking fundamentals rather than only meeting the tasks' requirements :)


