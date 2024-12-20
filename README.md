# arastat-core (under development)
ARA Applied Potentiostat Core. An API to generate DAC value for potentiostat. We use this API to generate DAC signals for cyclic voltammetry and LSV (untested) in our Potentiostat, ARASTAT. Our potentiostat approach design utilises differential amplifier in which the voltage applied from digital-to-analog converter would be subtracted with a reference voltage in the substracting amplifier.

ARAStat Schematics:
<img width="1233" alt="Screenshot 2024-12-21 at 06 52 14" src="https://github.com/user-attachments/assets/4bafa185-24be-43d5-96a4-20aa2afebe51" />



Here are some references we use to design and develop our potentiostat:

Crespo, J. R., Elliott, S. R., Hutter, T., & Águas, H. (2021). Development of a low-cost Arduino-based potentiostat. <a href=https://repositories.lib.utexas.edu/server/api/core/bitstreams/3e70ae06-19ce-4752-a574-c1441e6bb971/content>(link)</a><br>
Meloni, G. N. (2016). Building a microcontroller based potentiostat: A inexpensive and versatile platform for teaching electrochemistry and instrumentation. <a href=https://pubs.acs.org/doi/10.1021/acs.jchemed.5b00961>(link)</a>

Our design substracts the DAC signal voltage with half of the reference voltage such that we can use avaialble fixed voltages such as 5V and 3.3V. 

Feel free to use or modify it depending on your case.
