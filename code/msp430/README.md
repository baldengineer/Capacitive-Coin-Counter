You'll need TI's CAPTIVATE Design Center and Code Composer Studio.

1. Create a panel design in Design Studio with a single button, proximity detect, and the MSP430FR2633. 
2. In Design Center, Auto-Configure the pins, but then change the button to use CAP2.0 and CAP0.0.
3. Export this project to code composer studio
4. Import in Code Composer Studio (you can't just open it)
5. Replace main.c with this one.
6. Copy CAPT_UserConfig.c and CAPT_UserConfig.h to captivate_config

I NEVER got the "update existing project" to work in Design Center. So any changes you make there, make in the CAPT_UserConfig.h file.