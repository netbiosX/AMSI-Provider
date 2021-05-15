# AMSI-Provider

A fake AMSI Provider which can be used to gain persistence on a host when a specific text is triggered. By default a calc.exe will open.

# Credits

Originally this technique was discovered by [b4rtik](https://twitter.com/b4rtik) and more details can be found in the [article](https://b4rtik.github.io/posts/antimalware-scan-interface-provider-for-persistence/) on his blog. The code sample of the AMSI provider is courtesy of [Microsoft](https://docs.microsoft.com/en-us/samples/microsoft/windows-classic-samples/iantimalwareprovider-sample/) and the modifications of the code to [b4artik](https://twitter.com/b4rtik). Since the original code shared was missing some required headers and some functions were not defined I decided to put all of them in a single repository for easy usage.

