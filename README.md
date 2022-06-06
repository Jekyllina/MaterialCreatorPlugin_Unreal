# MaterialCreatorPlugin_Unreal
 Plugin to create a material starting from a txt file

The file txt must be in the Content folder. To start the plugin, write "creatematerials" and the name of the txt file in the Unreal Console Command, example: 
"creatematerials materials.txt". 
The materials will be created in a folder Materials in the Content browser.

You can specify a name for the material, if that name already exists a _ followed by a number will be added to the name; if you don't write a name, it will be Material_ 
with a number.
The parameters that you can decide for the material are:
- Color (RGB), if you don't specify the alpha it will be 255
- Emissive (RGB), if you don't specify the alpha it will be 255, and a multiplier for the intensity of light
- Texture, you have to specify the name and the texture must be in the Content folder
- Roughness value
- Metallic value

txt example:
![materials_txt](https://user-images.githubusercontent.com/48209077/172207780-008f435f-774b-4eab-89ee-8393edc02c32.jpg)

materials result examples:
simple material
![simpleMaterial](https://user-images.githubusercontent.com/48209077/172207879-07ee56a0-357b-43c3-820a-744e53e65843.jpg)

emissive material:
![EmissiveMaterial](https://user-images.githubusercontent.com/48209077/172208078-33161a56-05f7-454f-a7b3-34f08094dff5.jpg)

rough/metallic material:
![Rough-metallicMaterial](https://user-images.githubusercontent.com/48209077/172208143-579223d2-dcbb-46ae-bfaa-ed90b2e13bd9.jpg)
