<p><b>Do not build version v0 of the DG-Nova-4047-Disk-Adapter-Conn that plugs into the 4047. The pinout of the 50-pin connector is wrong on v0.</b></p>
These design files are the main elements to build a cable that connects from the 4047 Disk Adapter to a Nova 1200 computer. The connector on the back of the 4047 Disk Adapter is a 50-pin header with non-standard 0.15-inch pin spacing. The DG-Nova-4047-Disk-Adapter-Conn is a custom connector board that plugs onto that non-standard 50-pin header. The DG-Nova-CPU-Side-Disk-Adapter-Conn is a small connector board that mounts in the connector bay on the back of the Nova 1200 chassis. A pair of 26-pin flat cables connect these two boards. The wire-wrap pins on the two 26-pin headers on the DG-Nova-CPU-Side-Disk-Adapter-Conn must be connected to specific pins on the board slot on the Nova 1200 backplane where the 4046 Disk Controller is installed. The file "Cable 4046 to 4047 v01.xlsx" is a wirelist that defines the wire-wrap connections between the DG-Nova-CPU-Side-Disk-Adapter-Conn board and the slot with the 4046 Disk Controller.

The cable has a DG-Nova-4047-Disk-Adapter-Conn at one end and the DG-Nova-CPU-Side-Disk-Adapter-Conn at the other end.

<b>The first group of photos show details regarding the DG-Nova-4047-Disk-Adapter-Conn that plugs into the 4047.</b>

DG-Nova-4047-Disk-Adapter-Conn component side:
<img width="1800" height="806" alt="image" src="https://github.com/user-attachments/assets/96756fe5-aaee-424e-89a8-9761115f4e09" />

DG-Nova-4047-Disk-Adapter-Conn circuit side:
<img width="1800" height="681" alt="image" src="https://github.com/user-attachments/assets/97c80dfc-7c52-4f63-ac88-f9c4e49fa776" />

DG-Nova-4047-Disk-Adapter-Conn, with Disk Adapter Connector Spacer installed:
<img width="2858" height="930" alt="image" src="https://github.com/user-attachments/assets/9a7b2031-f845-4c59-8338-a6f41689b2a5" />

DG-Nova-4047-Disk-Adapter-Conn connected to "P2" on the back of the DG 4047 chassis:
<img width="2016" height="1512" alt="image" src="https://github.com/user-attachments/assets/ef6b9b73-8a05-414d-b04b-b8516958b895" />

<b>The second group of photos show details regarding the DG-Nova-CPU-Side-Disk-Adapter-Conn that's installed in the Nova 1200 CPU.</b>

DG-Nova-CPU-Side-Disk-Adapter-Conn component side:
<img width="1495" height="948" alt="image" src="https://github.com/user-attachments/assets/aced80cb-e6d7-4c62-beec-2a0ed1d82cc8" />

DG-Nova-CPU-Side-Disk-Adapter-Conn circuit side:
<img width="1734" height="1183" alt="image" src="https://github.com/user-attachments/assets/c6c8c524-6eec-444c-a7ef-00c595b7c95d" />

DG-Nova-CPU-Side-Disk-Adapter-Conn installed in the connector bay in a Nova 1200 CPU:
<img width="1134" height="1512" alt="image" src="https://github.com/user-attachments/assets/2a5b3fe9-962d-4670-a5da-1e581f0f794b" />

Photo showing wire-wrap connections (the green wires) from the DG-Nova-CPU-Side-Disk-Adapter-Conn to the slot with the 4046 Disk Controller. The 4046 is installed in the top slot in this chassis.
<img width="1134" height="1512" alt="image" src="https://github.com/user-attachments/assets/a958d5c0-0370-4b75-83ab-4383968ae779" />
