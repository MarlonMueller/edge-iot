# LoRa interconnection

The different devices shall be interconnected using 
[LoRa protocol](https://lora-developers.semtech.com/documentation/tech-papers-and-guides/lora-and-lorawan/).
[LoRa best practices](https://lora-developers.semtech.com/documentation/tech-papers-and-guides/the-book/packet-size-considerations/)
have been followed. 

The following fields (along with the number of bits used) are proposed to send
for each LoRa package. The bits occupied for each field is indicated in 
square brackets:

- [0, 47]. 48 bits reserved for ESP ID. 
- [48, 59]. 12 bits reserved for year as base 2 number. 
- [60, 63]. 4 bits reserved for month as base 2 number.
- [64, 68]. 5 bits reserved for hour as base 2 number.
- [69, 74]. 6 bits reserved for minute as base 2 number.
- [75, 80]. 6 bits reserved for second as base 2 number.
- [81, 82]. 2 bits reserved for neural network output.
- [83, 98]. 16 bits reserved for checksum. 

Therefore, a total of 99 bytes are required. A $DataRate = 0$ may be chosen 
(maximum data payload of $51$ bytes, or $408$ bits, according to 
EU regulations). 



