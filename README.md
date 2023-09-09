# TCP-Congestion-Control
The aim of this project is to get acquainted with the function of `TCP` and to implement the mechanism of sending correct information along with **congestion control** using `UDP` sockets in the network. In the first part we have the following topology:

In this topology, computer `A` sends a relatively large file to computer `B` through router `R`. Router `R` has a buffer in which incoming messages are stored and sent to the destination in `FIFO` form. Computers `A` and `B` and router `R` are considered independent processes. To send the file, the computer sends it in 1.5 KB packets through the **sliding-window** mechanism. `UDP-type` socket is used for the mentioned inter-process communication.

In the second part, we have 20 computers similar to computer `A` that send 1.5 KB packets to computer `B` through router `R`. In addition to the **Sliding-Window** mechanism, we implement the **Random Early Drop** congestion control mechanism to control network congestion.

## Go Back N Protocol (GBN)
Go-Back-N ARQ is a specific instance of the automatic repeat request (ARQ)
protocol, in which the sending process continues to send a number of frames
specified by a window size even without receiving an acknowledgment (ACK)
packet from the receiver.


## Selective Repeat Protocol (SR)
Selective repeat protocol, also called Selective Repeat ARQ (Automatic Repeat request), is a data link layer protocol that uses a sliding window method for
the reliable delivery of data frames. Here, only the erroneous or lost frames are
retransmitted, while the good frames are received and buffered.

## Random Early Drop
Rather than wait for queue to become full, drop each arriving packet with some drop probability whenever the queue length exceeds some drop level.
Here is the algorithm of RED:

