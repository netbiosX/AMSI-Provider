# AMSI-Provider

A fake AMSI Provider which can be used to gain persistence on a host when a specific text is triggered. By default calc.exe will open. 

# Usage

The AMSI Provider can be registered with the system by executing the following command from an elevated command prompt:

`regsvr32 AmsiProvider.dll`

Executing the following from a PowerShell console will open calc.exe:

`"pentestlab"`

![image](https://github.com/netbiosX/AMSI-Provider/blob/main/Calc.PNG)

# Credits

Originally this technique was discovered by [b4rtik](https://twitter.com/b4rtik) and more details can be found in the [article](https://b4rtik.github.io/posts/antimalware-scan-interface-provider-for-persistence/) on his blog. The code sample of the AMSI provider is courtesy of [Microsoft](https://docs.microsoft.com/en-us/samples/microsoft/windows-classic-samples/iantimalwareprovider-sample/) and the modifications of the code to [b4artik](https://twitter.com/b4rtik). Since the original code shared was missing some required headers and some functions were not defined I decided to put all of them in a single repository for easy usage.

