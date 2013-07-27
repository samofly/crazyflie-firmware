### REST API plan

It would be extremely useful, if user-space software would be able to control Crazyflie via a REST API.

The proposed architecture:

1. Host will have a daemon dealing with USB protocol and exposing an HTTP REST API on localhost (with a predefined port) or even provide a virtual network interface (on Linux).

This daemon will be the only part that requires root prigileges. The user programs will be simple REST clients
(may be even browser-based).

2. Crazyflie will have a new radio protocol implemented. At first, it will be similar to what we have now
(CRTP, CrazyRealtimeTransferProtocol). Later one of the following approaches will be adoped:

* HTTP/TCP/IP stack. Crazyflie will run a web server with REST API and the host daemon will be a tiny proxy.
* UDP and REST-over-UDP. This will make it harder to catch the telemetry streams, but easier to implement.
* HTTP over SCTP. This option needs more exploration. See also [Comparison of HTTP over TCP and SCTP](http://www.eecis.udel.edu/~leighton/firefox.html)
* SPDY/QUIC/UDP stack. This looks the most advanced option, because QUIC and SPDY know about ecnryption, streams, their priorities, work good in unrealiable and high-latency networks, and it's trivial to implement REST on top of SPDY. At the same time, it's the most controverial approach: QUIC is in an early stage of development, encryption on Cortex M3 will eat a lot of precious CPU cycles, 20 KB of RAM and 128 KB of Flash is probably not enough for all this complexity.


