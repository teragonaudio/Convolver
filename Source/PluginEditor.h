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

#ifndef __JUCER_HEADER_CONVOLVERAUDIOPROCESSOREDITOR_PLUGINEDITOR_49B0FCAC__
#define __JUCER_HEADER_CONVOLVERAUDIOPROCESSOREDITOR_PLUGINEDITOR_49B0FCAC__

//[Headers]     -- You can add your own extra header files here --
#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ConvolverAudioProcessorEditor  : public AudioProcessorEditor,
                                       public ButtonListener
{
public:
    //==============================================================================
    ConvolverAudioProcessorEditor (ConvolverAudioProcessor* ownerFilter);
    ~ConvolverAudioProcessorEditor();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    TextEditor* loadedFileTextEditor;
    TextButton* loadFileButton;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConvolverAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCER_HEADER_CONVOLVERAUDIOPROCESSOREDITOR_PLUGINEDITOR_49B0FCAC__
