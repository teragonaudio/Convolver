/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  24 Mar 2013 12:50:48pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "PluginEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ConvolverAudioProcessorEditor::ConvolverAudioProcessorEditor (ConvolverAudioProcessor* ownerFilter)
    : AudioProcessorEditor(ownerFilter),
      loadedFileTextEditor (0),
      loadFileButton (0)
{
    addAndMakeVisible (loadedFileTextEditor = new TextEditor ("Loaded File Text Editor"));
    loadedFileTextEditor->setMultiLine (false);
    loadedFileTextEditor->setReturnKeyStartsNewLine (false);
    loadedFileTextEditor->setReadOnly (true);
    loadedFileTextEditor->setScrollbarsShown (true);
    loadedFileTextEditor->setCaretVisible (true);
    loadedFileTextEditor->setPopupMenuEnabled (false);
    loadedFileTextEditor->setText ("No file loaded");

    addAndMakeVisible (loadFileButton = new TextButton ("Load File Button"));
    loadFileButton->setButtonText ("Load File");
    loadFileButton->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ConvolverAudioProcessorEditor::~ConvolverAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    deleteAndZero (loadedFileTextEditor);
    deleteAndZero (loadFileButton);


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ConvolverAudioProcessorEditor::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ConvolverAudioProcessorEditor::resized()
{
    loadedFileTextEditor->setBounds (16, 16, 408, 24);
    loadFileButton->setBounds (440, 16, 150, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ConvolverAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == loadFileButton)
    {
        //[UserButtonCode_loadFileButton] -- add your button handler code here..
        FileChooser fileChooser("Select Impulse Response", File::getSpecialLocation(File::userHomeDirectory));
        if (fileChooser.browseForFileToOpen()) {
            File impulseResponseFile(fileChooser.getResult());
            listener->onImpulseResponseSelected(impulseResponseFile);
        }
        //[/UserButtonCode_loadFileButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ConvolverAudioProcessorEditor"
                 componentName="" parentClasses="public AudioProcessorEditor"
                 constructorParams="ConvolverAudioProcessor* ownerFilter" variableInitialisers="AudioProcessorEditor(ownerFilter)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330000013"
                 fixedSize="1" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TEXTEDITOR name="Loaded File Text Editor" id="bec9aa2afbbc492" memberName="loadedFileTextEditor"
              virtualName="" explicitFocusOrder="0" pos="16 16 408 24" initialText="No file loaded"
              multiline="0" retKeyStartsLine="0" readonly="1" scrollbars="1"
              caret="1" popupmenu="0"/>
  <TEXTBUTTON name="Load File Button" id="2c0b4b9d39aaa81a" memberName="loadFileButton"
              virtualName="" explicitFocusOrder="0" pos="440 16 150 24" buttonText="Load File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif



//[EndFile] You can add extra defines here...
//[/EndFile]
