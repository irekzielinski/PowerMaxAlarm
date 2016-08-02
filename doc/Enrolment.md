# The enrollment process

Our application (emulated Powerlink) needs a pin code in order to:
  * Get some information from the alarm.
  * Use the bypass command.
  * Use the disarm and arm commands.
  
During the enrollment process the Powerlink will create a pin and register this pin at the alarm. 
The advantage is that none of the existing pins is required for the app to operate.

## Powermax+ and Powermax Pro
Enrolment can be done via the installer menu. When app is connected, enter installer menu and use 'Install Powerlink' option.
When the enrollment process is completed successfully, a beep will be sounded.

Note: When a new Powerlink module needs to be registered while there is already a Powerlink registered, the previous one needs to be uninstalled first. You can do so by selecting 'Install Powerlink' from the installer menu then pressing the disarm button.
        
## Powermax Complete
Enrolment is triggered without any need for user interaction.
Internally the application that uses [PMAX library](../README.md#generic-c-library-pmax) will ask to start download, this will fail on access denied, and PM will ask for enrolment.
