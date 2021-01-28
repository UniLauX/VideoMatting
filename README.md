# VideoMatting

The design and implementation of interactive video cutout system. <br>
Bachelor Thesis of [Yu Liu](https://sites.google.com/site/yuliuunilau/home) in Software Engineering, Southwest University. <br>
Supervisor: [Guofeng Zhang](http://www.cad.zju.edu.cn/home/gfzhang/)(Zhejiang University), Kaijin Qiu(Southwest University).

## Thesis and Demo
[Bachelor Thesis](https://drive.google.com/file/d/1gKGaUhhrVTZFG1N7YknHF86F6KBQb8Zv/view)
Video Demo


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
Visual Studio 2005, MFC Framework
C++
OpenCV 2.4.10 and CxImage

#### Acknowledgement: 
This work is done by Yu Liu was a Internship at State Key Lab of CAD and CG, work with Prof. Guofeng Zhang. The prototype software was based on some implementation from Computer Vision Group of  State Key Lab of CAD and CG, Zhejiang University.


#### Reference:
@article{rother2004grabcut,
  title={" GrabCut" interactive foreground extraction using iterated graph cuts},
  author={Rother, Carsten and Kolmogorov, Vladimir and Blake, Andrew},
  journal={ACM transactions on graphics (TOG)},
  volume={23},
  number={3},
  pages={309--314},
  year={2004},
  publisher={ACM New York, NY, USA}
}

@article{bai2009video,
  title={Video snapcut: robust video object cutout using localized classifiers},
  author={Bai, Xue and Wang, Jue and Simons, David and Sapiro, Guillermo},
  journal={ACM Transactions on Graphics (ToG)},
  volume={28},
  number={3},
  pages={1--11},
  year={2009},
  publisher={ACM New York, NY, USA}
}
