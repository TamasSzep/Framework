# Framework
3d framework for OpenGL, DirectX11 and DirectX12 rendering

![GitHub Logo](/Documents/git_image.png)
## Setup
* Unpack the following ZIPs with Total Commander:
    * Create a folder 'External' in Framework root
	* External.zip -> External (copy into)
	* External2.zip -> External (copy into)
	* Resources_Fonts.zip -> Resources
	* Resources_Models.zip -> Resources
	* Resources_Textures.zip -> Resources
* Build Project/VS2019/Framework2/Timeborne.sln with Visual Studio 2019.
* Run FrameworkTest. In FrameworkTest.cpp an enum switches between the OpenGL, DirectX11 and DirectX12 renderings.
## FAQ
* With DirectX11 the graphics device could not be created in debug configuration: try removing D3D11_CREATE_DEVICE_DEBUG when creating the graphics device, e.g. by commenting it out in CreateGraphicsDevice(...).
