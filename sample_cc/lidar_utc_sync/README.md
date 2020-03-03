# LiDAR UTC Time Synchronization

LiDAR UTC time synchronization demo shows how to synchronize time data (GPRMC/GNRMC) from serial port to Livox LiDAR.

## 1. Set Serial Port Name

Serial port name indicates where to read time data. Serial port name is set to : (1) `COM3` on Windows; (2) `/dev/ttyUSB0` on Linux by default. you can call `SetPortName (std::string port_name)` to set your synchronizer's serial port name.

## 2. Set Baud Rate

Baud rate is set to `BR9600` (9600) by default，you can call `SetBaudRate (BaudRate baudrate)` to set baud rate：

| BaudRate  |
| --------- |
| BR2400    |
| BR4800    |
| BR9600    |
| BR19200   |
| BR38400   |
| BR57600   |
| BR115200  |
| BR230400  |
| BR460800  |
| BR500000  |
| BR576000  |
| BR921600  |
| BR1152000 |
| BR1500000 |
| BR2000000 |
| BR2500000 |
| BR3000000 |
| BR3500000 |
| BR4000000 |

## 3. Set Parity

Parity is set to `P_8N1`(No parity) by default，you can call `SetParity(Parity parity)` to set parity :

| Parity                                                       |
| ------------------------------------------------------------ |
| P_8N1   /* No parity (8N1)	*/                             |
| P_7E1   /* Even parity (7E1)*/                               |
| P_7O1    /* Odd parity (7O1)	*/                           |
| P_7S1     /* Space parity is setup the same as no parity (7S1)	*/ |

**Note**

​	Livox-SDK release version <= 2.0.0 doesn't support to synchronize Mid40. It will be fix in next release.

​ Or you can refer to [Mid40's UTC synchronization](https://github.com/Livox-SDK/Livox-SDK/issues/31) to resolve the problem temporarily.

