# kra-format

In essence any *.kra*-file is a zipped archive wihch contains the layers in binary format. When unzipping such a file, 
the following file structure is found (using KRAExample.kra as an example):

```
KRAExample/
- preview.png
- mimetype
- mergedimage.png
- maindoc.xml
- documentinfo.xml
- KRAExample/
- - annotations/
- - - ...
- - layers/
- - - layer2
- - - layer2.defaultpixel
- - - layer2.icc
- - - ...
```

Of most importance are the 'maindoc.xml' file, containing information on document and layer properties and 
the 'layer2' binary file, which contains the compressed pixel data

Krita saves the layer in a special format that might, at first glance, not be obvious. Let's take for example a layer named "green" which, when seen in Krita has following form and placement on the image canvas:

![Tile Visual Structure](tile-visual-structure.png?raw=true "Tile Visual Structure")

As can be seen, the layer boundaries extend outside of the image canvas. Also not all parts of the image canvas contain a part of the "green" layer.

In the case of Krita, the image is subdivided in different regions called "tiles" which, by default, have dimensions 64 px x 64 px.
Each of these tiles is defined by its topleft coordinate in the image. When saving, Krita takes each of the layers and ONLY saves the tiles that contain any layer data. In the case of this example Krita would save the 5 tiles with coordinates: [0, 0], [64, 0], [0, 64], [64, 64] and [64, 128]. Thus even though some tiles might only contain a single pixel they are still saved regardless.

Importing this tile data starts from the maindoc.xml as the correct paths of each of the binary data of these layers is to be found first.

## 'maindoc.xml'

In the case of the KRAExample.kra-file this XML-document looks as follows:
```
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE DOC PUBLIC '-//KDE//DTD krita 2.0//EN' 'http://www.calligra.org/DTD/krita-2.0.dtd'>
<DOC xmlns="http://www.calligra.org/DTD/krita" kritaVersion="4.2.8" editor="Krita" syntaxVersion="2">
 <IMAGE proofing-depth="U8" y-res="100" proofing-profile-name="Chemical proof" description="" proofing-adaptation-state="1" proofing-intent="3" width="256" x-res="100" colorspacename="RGBA" mime="application/x-kra" proofing-model="CMYKA" name="KRAExample" height="128" profile="sRGB-elle-V2-srgbtrc.icc">
  <layers>
   <layer channellockflags="1111" locked="0" filename="layer2" channelflags="" onionskin="0" intimeline="1" y="0" colorlabel="0" visible="1" uuid="{70f61a54-32fb-4fd7-9418-6c69860450ac}" colorspacename="RGBA" nodetype="paintlayer" compositeop="normal" collapsed="0" opacity="255" name="clouds" x="0"/>
   <layer channellockflags="1111" locked="0" filename="layer3" channelflags="" onionskin="0" intimeline="1" y="0" colorlabel="0" visible="1" uuid="{683fcc00-dd23-4de2-9559-1ba917d53a7b}" colorspacename="RGBA" nodetype="paintlayer" compositeop="normal" collapsed="0" opacity="255" name="grass" x="0"/>
   <layer channellockflags="1111" locked="0" filename="layer4" channelflags="" onionskin="0" intimeline="1" y="0" colorlabel="0" visible="1" uuid="{7cc20c30-71ed-4c9b-8612-76bb2cae146e}" colorspacename="RGBA" nodetype="paintlayer" compositeop="normal" collapsed="0" opacity="255" name="ground" x="0"/>
   <layer channellockflags="1111" locked="0" filename="layer5" channelflags="" onionskin="0" intimeline="1" y="0" colorlabel="0" visible="1" uuid="{0e194a61-37b5-4f3d-a90a-0eed4ed37418}" colorspacename="RGBA" nodetype="paintlayer" compositeop="normal" collapsed="0" opacity="255" name="sun" x="0"/>
   <layer filename="layer6" compositeop="normal" colorspacename="RGBA" collapsed="0" name="Background" visible="1" selected="true" intimeline="1" y="0" channelflags="" colorlabel="0" uuid="{78775104-9586-4774-9119-36a5640cc1af}" x="0" nodetype="paintlayer" onionskin="0" channellockflags="1111" locked="1" opacity="255"/>
  </layers>
  <ProjectionBackgroundColor ColorData="AAAAAA=="/>
  <GlobalAssistantsColor SimpleColorData="176,176,176,255"/>
  <ProofingWarningColor>
   <RGB space="sRGB-elle-V2-srgbtrc.icc" r="0" b="0" g="1"/>
  </ProofingWarningColor>
  <Palettes/>
  <animation>
   <framerate type="value" value="24"/>
   <range from="0" to="100" type="timerange"/>
   <currentTime type="value" value="0"/>
  </animation>
 </IMAGE>
</DOC>
```

Of particular interest are the attributes of the IMAGE element and the layers contained in the layers range. Each layer contains a **filename** attribute which gives the path to the compressed binary data of this layer which will have to be parsed and uncompressed.

## 'layer2'

Each binary layer file of the KRA archive obeys following format (values are purely illustrational):

![Tile Binary Structure](tile-binary-structure.png?raw=true "Tile Binary Structure")

As is evident, this layer contains a number of tiles, each having a single line header and a main header that defines the uncompressed data and the format. Each of these lines is terminated with an LF (Line Feed) byte, having a hex-value of 0x0A. 

### Layer header

Following attributes are present in the main header:
- **VERSION**: version statement of the saved file, in most cases this will be 2
- **TILEWIDTH**: width of the saved tiles, by default this will be 64
- **TILEHEIGHT**: height of the saved tiles, by default this will be 64
- **PIXELSIZE**: number of bytes stored for each pixel, RGBA requires 4 bytes
- **DATA**: number of tiles that are stored for this layer

An important value which can be derived from this is the uncompressed data byte size which will be equal to:  
`UNCOMPRESSEDSIZE = PIXELSIZE*TILEWIDTH*TILEHEIGHT`

### Tile header

Each of the tiles also contains a small header with following attributes:
- **TILE_LEFT,TILE_TOP,LZF,COMPRESSEDSIZE**: thus giving an indication of the compression algorithm (which is pretty much always LZF), the number of bytes that will follow after this header and the topleft coordinates of the tile that these bytes represent.
- **0** or **1**: 0 indicates that the data of this tile is uncompressed, while 1 indicates that this tile's data is compressed.

The data that follows can than be uncompressed using the LZF algorithm as found here on the Krita main repository:  
https://invent.kde.org/kde/krita/-/blob/master/libs/image/tiles3/swap/kis_lzf_compression.cpp

The resulting uncompressed data will obey following format (in the case of RGBA):  
`B_1, B_2, ..., B_end, G_1, G_2, ..., G_end, R_1, R_2, ..., R_end, A_1, A_2, ..., A_end`

So the Red and Blue channels are swapped for some reason and format will have to be sorted to be applicable to ImageMagick's Image class.

More documentation regarding the KRA format might be written whenever additional functionality is required.
