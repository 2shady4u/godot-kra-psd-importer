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

## maindoc.xml

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

In the codebase this XML-file is used to create a **KraDocument** struct which contains following properties:
- **width**: the width of image canvas of the *.kra*-file.
- **height**: the width of image canvas of the *.kra*-file.
- **channelCount**: the number of color channels stored in the KRA Archive which is equal to 4 when using RGBA.
- **name**: the name of the *.kra*-file.
- **layers**: a vector of layers stored in the KRA archive.

Each of the KraLayers, as found in the layers-vector, are also structs that contain following variables:
- ...
- **tiles**: a vector of tiles that make up the layer.

Each of tiles are stored, in compressed form, in the layer2 binary file and will have to be extracted, one-by-one, in a
sequential manner.

## layer2

Contains the binary data of all the tiles of layer2 in a LZF compressed format.
