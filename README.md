Direct3D Invisible Walls
========================

Using Deviare to Cheat on Games: Intercepting Direct3D COM objects and making walls invisible.

This simple Deviare code allows you to to see through game walls. We intercept Direct3D objects and select wireframe mode so the walls are transparent. This code injects a DLL to create an object based on the IDirect3D9 interface and hook the address of the CreateDevice COM method from the virtual table.

[Original article](http://blog.nektra.com/main/2013/07/01/using-deviare-to-cheat-on-games-intercepting-direct3d-com-objects-and-making-walls-invisible/)
