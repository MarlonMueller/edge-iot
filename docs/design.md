# Project design (DRAFT)

## Overview

TBD

## Requirements

Our project operates within a specific context where various edge devices must be deployed in low-energy resource environments. 
Therefore, we have to be very careful with the resources we use. Additionally, as we are covering vast distances, 
we also need to select the optimal protocol capable of both extensive reach and minimal energy consumption. In summary, our 
key requirements are:

- **Resource-Efficient Devices**: We require edge devices that minimize energy consumption while having enough memory for hosting 
  a Neural Network model inside it, along with the necessary libraries to execute them effectively.

- **Efficient Protocols**: The chosen communication protocols should be cover large distances while consuming as little energy as possible.

## Architecture

The architectures that have been used in the state-of-the-art (the papers may be found 
[here](https://github.com/MarlonMueller/edge-iot/wiki/Literature) and also in the report) are various. From all of the different
architectures that can be found, we may highlight the following ones:

- In [(Höchst et al., 2022)](https://doi.org/10.1007/978-3-031-17436-0_6), the authors propose a system architecture that
  consists of different ESP32 with integrated microphones to capture data, which is then wirelessly transmitted 
  via Wi-Fi to the central node, an NVIDIA Jetson Nano. In this architecture, the ESP32 transmit all the data captured directly
  to the central node, which is responsible for the data processing and the inference of the Neural Network model, which presents
  the flaw that the central node is overloaded with work. Also, the protocol that is used is Wi-Fi, which is not the most
  energy-efficient protocol (in fact, even using batteries of 3.500 mAh for the microphones on the edge, they barely achieve 
  1 day of lifespan). 

- In [(Fengbo et al, 2018)](https://doi.org/10.1109/SNSP.2018.00017), the proposed architecture for a case of monitoring fire within
  a forest is several [CC2530](https://www.ti.com/lit/ds/symlink/cc2530.pdf?ts=1699268198424) devices as edge nodes, which transfer
  environmental information to a central node using Zigbee protocol. In this case, the used communication protocol is energy-efficient 
  compared to technologies such as Wi-Fi, but the edge nodes don't have the ability to process a Neural Network model (to our knowledge). 

- In [(Singh et al., 2022)](https://doi.org/10.1016/j.jksuci.2021.02.009), the proposed architecture for an use case of forest's monitoring
  consists of edge devices (not specificied in paper) interconnected using LoRA. No further information is given as this paper just
  proposes a system architecture, but the usage of LoRA is interesting as it is a protocol that is even more efficient than Zigbee and
  cover larger distances. 

Therefore, taking into consideration the requirements and constraints we have in our use case, as well as the advantages and disadvantages
of the different architectures founded in the state-of-the-art, we propose the following architecture: 

![Architecture Design](../assets/architecture_design.png)

We propose a centralized architecture that consists of ESP32 in edge nodes, each one of them equipped with a microphone used to
record environmental sounds. The ESP32 are connected to a Raspberry Pi (central node) using LoRA protocol. When an ESP32 detects a specific bird
species through its microphone, it starts to transmit the data to the Raspberry Pi. The Raspberry Pi gathers all the information
received from the different ESP32 and processes meaningful information from them, such as the location of the bird species and
the time when it was recorded. This information is then sent to the Cloud, where it is stored 
and used for further analysis. The edge nodes are powered using batteries, whereas the central node may be strategically positioned 
for access to the power grid. 

Compared to the state-of-the-art, we have used ESP32 as in [(Höchst et al., 2022)](https://doi.org/10.1007/978-3-031-17436-0_6), as
this one of the lowest power consumption microcontrollers that we have found with enough memory to fit a neural network inside it and with
a dedicated library to introduce a pretrained neural network within it. Also, as motivated in 
[(Singh et al., 2022)](https://doi.org/10.1016/j.jksuci.2021.02.009), we have used LoRA as the communication protocol, because of 
its exceptional energy efficiency and long-range coverage, and to our knowledge it is the best suited protocol for our use case.
This protocol aligns with our project's requirements, as our data packets are minimal (only a few bytes per edge node detection).
Therefore, we expect that the energy consumption of our architecture is lower than 
in [(Höchst et al., 2022)](https://doi.org/10.1007/978-3-031-17436-0_6) and than in 
[(Fengbo et al, 2018)](https://doi.org/10.1109/SNSP.2018.00017), where Wi-Fi and Zigbee are used, respectively.