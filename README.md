# VideoMatting

The design and implementation of interactive video cutout system. 2013 <br>
[Yu Liu](https://sites.google.com/site/yuliuunilau/home)' bachelor thesis in Software Engineering, Southwest University. <br>
Supervisor: [Guofeng Zhang](http://www.cad.zju.edu.cn/home/gfzhang/)(Zhejiang University), Kaijin Qiu(Southwest University).

## Thesis and Demo
[Bachelor Thesis](https://drive.google.com/file/d/1gKGaUhhrVTZFG1N7YknHF86F6KBQb8Zv/view) <br>
[Software Video Demo](https://www.youtube.com/watch?v=sVaJevQcH-g&feature=youtu.be)

#### Abstract
As a vital research direction of computer vision, Video Cutout System can track and predict
motion of the object dynamically in video sequences. It has been widely used in many fields, such
as national defense,transportation, etc.
In the text, it mainly discuss about the design and Implementation of interactive video cutout
system. Firstly, the text describle and analysis the algorithm principle of GrabCut used in still
image and Video SnapCut in Video sequences in detail. Secondly, with the software way, GrabCut
be implemented to get the foreground object in still image, the system mainly based on Video
SnapCut, which is a popular video cutout algorithm, can be used for object-tracking and motion
estimated dynamically. Additionally, the system also provide a series tools for user interactive to
get a better result at real-time, all those work lead to a satisfied result. Moreover, the system also
make use of OpenMP for multithreaded program to get a better reaction speed, which is contribute
to enhancing the performance of our system. The focus of the text is algorithm principle and
design of Video SnaptCut, the interface is well designed and contain various content.
Key word: Video SnapCut; GrabCut; Video CutOut; Interactive; Multi-thread.


#### Environment
Visual Studio 2005, MFC Framework </br>
C++ <br>
OpenCV 2.4.10 and CxImage <br>

#### Acknowledgement: 
This work is done by Yu Liu was a Internship at State Key Lab of CAD and CG, work with Prof. Guofeng Zhang. The prototype software was based on some implementation from [Computer Vision Group](http://www.zjucvg.net/) of  State Key Lab of CAD and CG, Zhejiang University.


#### Cite:
The design and implementation of interactive video cutout system. Yu Liu, Guofeng Zhang, Kaijin Qiu, 2013
