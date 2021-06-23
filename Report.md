# EE6470 Final Project Report
###### tags: `EE6470`

## Problems and Solutions
* We will implement LaNet in 4 different ways.
    * Single core and no data reuse in conv/ fc layers
    * Single core and data reuse in conv/ fc layers
    * Multi core and no data reuse in conv/ fc layers
    * Multi core and data reuse in conv/ fc layers
* And using CIFAR-10 dataset
* Compare and analyze the results from above 4 different implementations.
* CIFAR-10 dataset consists of 60000 32*32 colour images in 10 classes.
* LaNet model
![](https://i.imgur.com/uYl0wrg.png)
* We do quantization in our model as well to reduce area and power consumption.
* Besides that we also implement data reuse in both convolution and fully connect layers.
![](https://i.imgur.com/Qy00Gzo.png)
![](https://i.imgur.com/hV31cR2.png)

## Implementation details 
### Design overview(single core)
![](https://i.imgur.com/zVGOF8I.png)
### Design overview(mult core)
![](https://i.imgur.com/AAvJjTM.png)
### Platform code
* It is our definition about memory address for convolution and fully connect layers

![](https://i.imgur.com/sJAQDUz.png)
![](https://i.imgur.com/qlwpOL9.png)

### convolution module

Below figures are the overall pictures of our design

![](https://i.imgur.com/muuHT3w.png)

As we can see from our code, our implementation is very straightforward. 

![](https://i.imgur.com/DGbrTrf.png)
![](https://i.imgur.com/EKk9VLE.png)

### fully connected module(no bias)

![](https://i.imgur.com/ka1MVvA.png)
![](https://i.imgur.com/kbo7FH4.png)

### fully connected module (with bias)

![](https://i.imgur.com/1TnjK9k.png)
![](https://i.imgur.com/8l8HP7p.png)

### software main

In main file we execute program sequential with order do_conv1 -> do_conv2 -> do_fc1 -> do_fc2 -> do_fc3 -> get result.
![](https://i.imgur.com/Vhio62C.png)

### software do_conv

![](https://i.imgur.com/wdG1xGK.png)


### software do_fc

![](https://i.imgur.com/Pg9ikpw.png)

### convolution module(data reuse)

![](https://i.imgur.com/XTl2Hai.png)
![](https://i.imgur.com/1CNl1qU.png)

### fully connected module(no bias)(data reuse)

In data reuse version we need to transmit start and end address.

![](https://i.imgur.com/SC0eYjV.png)
![](https://i.imgur.com/4WHIWS3.png)

### fully connected module(with bias)(data reuse)

![](https://i.imgur.com/fZXZrvm.png)
![](https://i.imgur.com/rygDjmD.png)

### do_conv (data reuse)
![](https://i.imgur.com/CM61J7J.png)


### do_fc (data reuse)
![](https://i.imgur.com/MueyoFS.png)


## Experimental results
### Area in HLS
For the non-reuse and resue version of our design, we can find that the reuse version would consume more 30%~60% area than non-resue version, the main reason is that reuse version use buffer to prevent replicated input, so it need more area to place buffer , buffer data path, and buffer controller to implement the acceleration.
![](https://i.imgur.com/TxIqvoC.png)
Here is the synthesised diagram in Stratus to show that there is exactly lots of area is used for buffer.
Non-Reuse
![](https://i.imgur.com/KKoFzmt.png)
Reuse
![](https://i.imgur.com/SYKAzYo.png)

### Latency in HLS
For the sumulation result of two methods, row-based(reuse) version still consume more times rather than basic(non-reuse) in fc layer, the reason is that the compuation is not so much, it can be neglected.
![](https://i.imgur.com/raaGnnc.png)


### Latency in RISC-V vp
We have done two comparison, first one is comparing two methods in different cores, which can save up to 20 % times, and the second one is comparing two different cores architecture in different methods, which can save up 90 % times.
![](https://i.imgur.com/LAePlgF.png)
![](https://i.imgur.com/XLnALlJ.png)
![](https://i.imgur.com/o1vgjDB.png)
![](https://i.imgur.com/QaWChf7.png)
Finally here is the overview figrue to show the calculating times. Which show that the acceleration of different hardware architecture has extremly effect.
![](https://i.imgur.com/MOIESRG.png)


## Discussions and conclusions
For the entrie system, we should not only consider the accelerator architecture, also need to consider the software algorithm. Wrong algorithm apply on some ASIC design can not have better performance. Also, I would like to try ESP to embbed my design into it, maybe in the summer vacation. Finnaly, We want to said that this course is one of the most usefule course that I study in NTHU. We truly appreciate the helping from Professor and TAs.
