# arastat-core (under development)
ARA Applied Potentiostat Core. A library to control the DAC for potentiostat. Our potentiostat approach design utilises differential amplifier in which the voltage applied from digital-to-analog converter would be subtracted with a reference voltage in the substracting amplifier.

Here are some references we use to develop our potentiostat:

Crespo, J. R., Elliott, S. R., Hutter, T., & Águas, H. (2021). Development of a low-cost Arduino-based potentiostat. <a href=https://repositories.lib.utexas.edu/server/api/core/bitstreams/3e70ae06-19ce-4752-a574-c1441e6bb971/content>(link)</a>

Our design substracts the DAC signal voltage with half of the reference voltage such that we can use avaialble fixed voltages such as 5V and 3.3V. 

Feel free to use or modify it depending on your case.
