/*
 *
 * This source code is available under the "Simplified BSD license".
 *
 * Copyright (c) 2013, J. Kleiner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the original author nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
package Main;


import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.Timer;
import Models.SiModel;
import Models.XyModel;
import Widgets.PropDispatcher;

/**
 * This class implements a simple spectral display and control application.
 * It is intended to be used with a BREC one or two stage IF configuration.
 * @author user
 */
public class InstSi extends javax.swing.JPanel {

    private SiModel                mSaModel;
    private XyModel                mXySpectrum;
    private PropDispatcher         mDispatcher;  
    private String                 mApplTitleStr;
    
    /**
     * Creates new form InstSi
     */
    public InstSi() {
                        
        // Setup the application title string 
        mApplTitleStr = "Si";
        
        // Create a property change dispatcher
        mDispatcher = new PropDispatcher();
        
        // Create the models
        mSaModel      = new SiModel();
        mXySpectrum   = new XyModel();
        
        // Register the sa model as the source of the xy models
        mXySpectrum.RegisterXySrc( mSaModel, 0 );

        // Create the primary xy display
        mXySpectrum.getXyDisplay().setXyCapacity(       4096);
        mXySpectrum.getXyDisplay().setMarkersVisible(   false);
        mXySpectrum.getXyDisplay().setMarkerActive(     9);
        mXySpectrum.getXyDisplay().setMarkerVisible(    9, true);
        mXySpectrum.getXyDisplay().setXlimits(          -22000, +22000);
        mXySpectrum.getXyDisplay().setXmajorTicks(      10);
        mXySpectrum.getXyDisplay().setYmajorTicks(      11);
        mXySpectrum.getXyDisplay().setXlabel(          "F(MHz)");
        mXySpectrum.getXyDisplay().setYlabel(          "dB");
        mXySpectrum.getXyDisplay().setTitleStr(        mApplTitleStr);
        mXySpectrum.getXyDisplay().setCaptionStr(      "caption1");
        mXySpectrum.getXyDisplay().setBackground(new java.awt.Color(255, 255, 255));
        // mXySpectrum.getXyDisplay().setXformat( 'E' );
        
        
        // Init the generated components
        initComponents();
        
        // Register the  models with dispatcher
        mDispatcher.RegisterModel(mSaModel, "si" );
        mDispatcher.RegisterModel(mXySpectrum, "spec" );
        
        // Register the component tree containing all buttons with dispatcher
        mDispatcher.RegisterComponentTree(mPanelLeftControls);
        mDispatcher.RegisterComponentTree(mPanelRightControls);
        
        //  Establish property change linkage with model
        mSaModel.addPropertyChangeListener(mDispatcher.GetPropListener());

        // Start the engines
        mSaModel.Start();  

        // Re-fire the models to ensure hmi components match current model
        mSaModel.FireAll();
        mXySpectrum.FireAll();

        // Add XY display reserved panel
        mPanelDisplay.add(mXySpectrum.getXyDisplay() );

        // Force set the scales after the buttons have been created
        // This will override any general model defaults to this 
        // application's custom defaults
        panelInstBtnRef.setValue(        "0");
        panelInstBtnDbPerDiv.setValue(  "10" );
        panelInstBtnSiEnv.setValue(     "OFF");
        panelInstBtnSiPeakPick.setValue("OFF");
        panelInstBtnSiPeakFrac.setValue("0.01");
        
        // Need a timer to extract data from engine, place in xy and repaint xy
        ActionListener taskPerformer = new ActionListener() {
    	   public void actionPerformed(ActionEvent evt) {             
               // Supply an action on swing thread for models to push updates
               mSaModel.UiUpdateRequest();
               mXySpectrum.UiUpdateRequest();
    	   } // End of timer action
        };
        Timer timer = new Timer( 30 , taskPerformer);
        timer.start();

    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        mPanelMain = new javax.swing.JPanel();
        mPanelUpper = new javax.swing.JPanel();
        mPanelLeftControls = new javax.swing.JPanel();
        mPanelLeftHolder = new javax.swing.JPanel();
        mPanelDevice = new javax.swing.JPanel();
        panelInstBtnSgState = new Widgets.InstButton();
        panelInstBtnSgStatus = new Widgets.InstButton();
        panelInstBtnSgDevAddr = new Widgets.InstButton();
        mPanelSweepCtl = new javax.swing.JPanel();
        panelInstBtnFc = new Widgets.InstButton();
        panelInstBtnFspan = new Widgets.InstButton();
        mPanelScale = new javax.swing.JPanel();
        panelInstBtnRef = new Widgets.InstButton();
        panelInstBtnDbPerDiv = new Widgets.InstButton();
        mPanelDisplayControl = new javax.swing.JPanel();
        panelInstBtnSiEnv = new Widgets.InstButton();
        panelInstBtnSiPeakPick = new Widgets.InstButton();
        panelInstBtnSiPeakFrac = new Widgets.InstButton();
        mPanelDisplay = new javax.swing.JPanel();
        mPanelRightControls = new javax.swing.JPanel();
        mPanelRightHolder = new javax.swing.JPanel();
        mPanelResolution = new javax.swing.JPanel();
        panelInstBtnMbw = new Widgets.InstButton();
        panelInstBtnNfft = new Widgets.InstButton();
        panelInstBtnAve = new Widgets.InstButton();
        panelInstBtnInt = new Widgets.InstButton();
        mPanelTg = new javax.swing.JPanel();
        panelInstBtnTgPoints = new Widgets.InstButton();
        mPanelGainAtten = new javax.swing.JPanel();
        panelInstBtnIfGain = new Widgets.InstButton();
        panelInstBtnRfChannel = new Widgets.InstButton();
        panelInstBtnCoarseAtten = new Widgets.InstButton();
        panelInstBtnFineAtten = new Widgets.InstButton();
        mPanelMemory = new javax.swing.JPanel();
        mPanelMemShow = new javax.swing.JPanel();
        mCheckBoxM0 = new javax.swing.JCheckBox();
        mCheckBoxM1 = new javax.swing.JCheckBox();
        mCheckBoxM2 = new javax.swing.JCheckBox();
        mCheckBoxM3 = new javax.swing.JCheckBox();
        mPanelMemStore = new javax.swing.JPanel();
        mButtonM0 = new javax.swing.JButton();
        mButtonM1 = new javax.swing.JButton();
        mButtonM2 = new javax.swing.JButton();
        mButtonM3 = new javax.swing.JButton();
        mPanelLower = new javax.swing.JPanel();

        setLayout(new javax.swing.BoxLayout(this, javax.swing.BoxLayout.LINE_AXIS));

        mPanelMain.setLayout(new javax.swing.BoxLayout(mPanelMain, javax.swing.BoxLayout.Y_AXIS));

        mPanelUpper.setLayout(new javax.swing.BoxLayout(mPanelUpper, javax.swing.BoxLayout.LINE_AXIS));

        mPanelLeftControls.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelLeftControls.setAlignmentY(0.0F);
        mPanelLeftControls.setLayout(new javax.swing.BoxLayout(mPanelLeftControls, javax.swing.BoxLayout.Y_AXIS));

        mPanelLeftHolder.setAlignmentX(0.0F);
        mPanelLeftHolder.setAlignmentY(0.0F);
        mPanelLeftHolder.setLayout(new javax.swing.BoxLayout(mPanelLeftHolder, javax.swing.BoxLayout.Y_AXIS));

        mPanelDevice.setBorder(javax.swing.BorderFactory.createTitledBorder("Device"));
        mPanelDevice.setLayout(new javax.swing.BoxLayout(mPanelDevice, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnSgState.setLabel("State");
        panelInstBtnSgState.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgState.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgState.setOptions(new String[] {"ON", "OFF"});
        panelInstBtnSgState.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgState.setValue("OFF");
        panelInstBtnSgState.setValueName("si-StateStr");
        mPanelDevice.add(panelInstBtnSgState);

        panelInstBtnSgStatus.setLabel("Status");
        panelInstBtnSgStatus.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgStatus.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgStatus.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgStatus.setValue("OK");
        panelInstBtnSgStatus.setValueName("si-StatusStr");
        mPanelDevice.add(panelInstBtnSgStatus);

        panelInstBtnSgDevAddr.setDialogInput("DialogString");
        panelInstBtnSgDevAddr.setLabel("Device");
        panelInstBtnSgDevAddr.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgDevAddr.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgDevAddr.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgDevAddr.setValue("192.168.0.2:8000");
        panelInstBtnSgDevAddr.setValueName("si-DevStr");
        mPanelDevice.add(panelInstBtnSgDevAddr);

        mPanelLeftHolder.add(mPanelDevice);

        mPanelSweepCtl.setBorder(javax.swing.BorderFactory.createTitledBorder("Sweep"));
        mPanelSweepCtl.setLayout(new javax.swing.BoxLayout(mPanelSweepCtl, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnFc.setToolTipText("Reserved");
        panelInstBtnFc.setDialogInput("DialogKeyPad");
        panelInstBtnFc.setDoubleBuffered(false);
        panelInstBtnFc.setLabel("Fc(MHz)");
        panelInstBtnFc.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnFc.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnFc.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnFc.setValue("88");
        panelInstBtnFc.setValueName("si-FcenterMHzStr");
        panelInstBtnFc.setVerifyInputWhenFocusTarget(false);
        mPanelSweepCtl.add(panelInstBtnFc);

        panelInstBtnFspan.setToolTipText("Nest");
        panelInstBtnFspan.setDialogInput("DialogKeyPad");
        panelInstBtnFspan.setDoubleBuffered(false);
        panelInstBtnFspan.setLabel("Span(MHz)");
        panelInstBtnFspan.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnFspan.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnFspan.setName(""); // NOI18N
        panelInstBtnFspan.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnFspan.setValue("99");
        panelInstBtnFspan.setValueName("si-SpanMHzStr");
        panelInstBtnFspan.setVerifyInputWhenFocusTarget(false);
        mPanelSweepCtl.add(panelInstBtnFspan);

        mPanelLeftHolder.add(mPanelSweepCtl);

        mPanelScale.setBorder(javax.swing.BorderFactory.createTitledBorder("Scale"));
        mPanelScale.setLayout(new javax.swing.BoxLayout(mPanelScale, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnRef.setToolTipText("Reserved");
        panelInstBtnRef.setDialogInput("");
        panelInstBtnRef.setLabel("Ref(dB)");
        panelInstBtnRef.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnRef.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnRef.setOptions(new String[] {"0", "-10", "-20", "-30", "-40", "-50", "-60", "-70"});
        panelInstBtnRef.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnRef.setValue("0");
        panelInstBtnRef.setValueName("spec-YrefStr");
        mPanelScale.add(panelInstBtnRef);

        panelInstBtnDbPerDiv.setToolTipText("Reserved");
        panelInstBtnDbPerDiv.setLabel("dB/Div");
        panelInstBtnDbPerDiv.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnDbPerDiv.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnDbPerDiv.setOptions(new String[] {"3", "6", "10", "15"});
        panelInstBtnDbPerDiv.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnDbPerDiv.setValue("0");
        panelInstBtnDbPerDiv.setValueName("spec-YdivStr");
        mPanelScale.add(panelInstBtnDbPerDiv);

        mPanelLeftHolder.add(mPanelScale);

        mPanelDisplayControl.setBorder(javax.swing.BorderFactory.createTitledBorder("Display"));
        mPanelDisplayControl.setLayout(new javax.swing.BoxLayout(mPanelDisplayControl, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnSiEnv.setLabel("Envelope");
        panelInstBtnSiEnv.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiEnv.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiEnv.setOptions(new String[] {"ON", "OFF"});
        panelInstBtnSiEnv.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiEnv.setValue("OFF");
        panelInstBtnSiEnv.setValueName("spec-EnvEnableStr");
        mPanelDisplayControl.add(panelInstBtnSiEnv);

        panelInstBtnSiPeakPick.setLabel("Peaks");
        panelInstBtnSiPeakPick.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiPeakPick.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiPeakPick.setOptions(new String[] {"ON", "OFF"});
        panelInstBtnSiPeakPick.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiPeakPick.setValue("OFF");
        panelInstBtnSiPeakPick.setValueName("spec-PeakEnableStr");
        mPanelDisplayControl.add(panelInstBtnSiPeakPick);

        panelInstBtnSiPeakFrac.setToolTipText("Reserved");
        panelInstBtnSiPeakFrac.setDialogInput("");
        panelInstBtnSiPeakFrac.setDoubleBuffered(false);
        panelInstBtnSiPeakFrac.setLabel("PeakFwx");
        panelInstBtnSiPeakFrac.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiPeakFrac.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiPeakFrac.setOptions(new String[] {"0.01", "0.03", "0.05", "0.07", "0.10"});
        panelInstBtnSiPeakFrac.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSiPeakFrac.setValue("88");
        panelInstBtnSiPeakFrac.setValueName("spec-PeakFracStr");
        panelInstBtnSiPeakFrac.setVerifyInputWhenFocusTarget(false);
        mPanelDisplayControl.add(panelInstBtnSiPeakFrac);

        mPanelLeftHolder.add(mPanelDisplayControl);

        mPanelLeftControls.add(mPanelLeftHolder);

        mPanelUpper.add(mPanelLeftControls);

        mPanelDisplay.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelDisplay.setAlignmentY(0.0F);
        mPanelDisplay.setLayout(new java.awt.GridLayout(0, 1));
        mPanelUpper.add(mPanelDisplay);

        mPanelRightControls.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelRightControls.setAlignmentY(0.0F);
        mPanelRightControls.setLayout(new javax.swing.BoxLayout(mPanelRightControls, javax.swing.BoxLayout.Y_AXIS));

        mPanelRightHolder.setLayout(new javax.swing.BoxLayout(mPanelRightHolder, javax.swing.BoxLayout.Y_AXIS));

        mPanelResolution.setBorder(javax.swing.BorderFactory.createTitledBorder("Resolution"));
        mPanelResolution.setLayout(new javax.swing.BoxLayout(mPanelResolution, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnMbw.setToolTipText("Reserved");
        panelInstBtnMbw.setDialogInput(""); // NOI18N
        panelInstBtnMbw.setDoubleBuffered(false);
        panelInstBtnMbw.setLabel("MBW(kHz)");
        panelInstBtnMbw.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnMbw.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnMbw.setOptions(new String[] {"450", "250", "125", "63"});
        panelInstBtnMbw.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnMbw.setValue("88");
        panelInstBtnMbw.setValueName("si-MbwStr");
        panelInstBtnMbw.setVerifyInputWhenFocusTarget(false);
        mPanelResolution.add(panelInstBtnMbw);

        panelInstBtnNfft.setToolTipText("Reserved");
        panelInstBtnNfft.setDialogInput(""); // NOI18N
        panelInstBtnNfft.setDoubleBuffered(false);
        panelInstBtnNfft.setLabel("FFTN");
        panelInstBtnNfft.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnNfft.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnNfft.setOptions(new String[] {"128", "256", "512", "1024", "2048", "4096", "8192"});
        panelInstBtnNfft.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnNfft.setValue("88");
        panelInstBtnNfft.setValueName("si-FftnStr");
        mPanelResolution.add(panelInstBtnNfft);

        panelInstBtnAve.setToolTipText("Reserved");
        panelInstBtnAve.setDialogInput(""); // NOI18N
        panelInstBtnAve.setDoubleBuffered(false);
        panelInstBtnAve.setLabel("AVE");
        panelInstBtnAve.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnAve.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnAve.setOptions(new String[] {"1", "2", "4", "8", "16"});
        panelInstBtnAve.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnAve.setValue("88");
        panelInstBtnAve.setValueName("si-AveStr");
        panelInstBtnAve.setVerifyInputWhenFocusTarget(false);
        mPanelResolution.add(panelInstBtnAve);

        panelInstBtnInt.setToolTipText("Reserved");
        panelInstBtnInt.setDialogInput(""); // NOI18N
        panelInstBtnInt.setDoubleBuffered(false);
        panelInstBtnInt.setLabel("INT");
        panelInstBtnInt.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnInt.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnInt.setOptions(new String[] {"ON", "OFF"});
        panelInstBtnInt.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnInt.setValue("88");
        panelInstBtnInt.setValueName("si-IntStr");
        panelInstBtnInt.setVerifyInputWhenFocusTarget(false);
        mPanelResolution.add(panelInstBtnInt);

        mPanelRightHolder.add(mPanelResolution);

        mPanelTg.setBorder(javax.swing.BorderFactory.createTitledBorder("Tracking"));
        mPanelTg.setLayout(new javax.swing.BoxLayout(mPanelTg, javax.swing.BoxLayout.LINE_AXIS));

        panelInstBtnTgPoints.setToolTipText("Reserved");
        panelInstBtnTgPoints.setDialogInput(""); // NOI18N
        panelInstBtnTgPoints.setDoubleBuffered(false);
        panelInstBtnTgPoints.setLabel("TGP");
        panelInstBtnTgPoints.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnTgPoints.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnTgPoints.setOptions(new String[] {"0", "64", "128", "256", "512", "1024"});
        panelInstBtnTgPoints.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnTgPoints.setValue("88");
        panelInstBtnTgPoints.setValueName("si-TgPointsStr");
        panelInstBtnTgPoints.setVerifyInputWhenFocusTarget(false);
        mPanelTg.add(panelInstBtnTgPoints);

        mPanelRightHolder.add(mPanelTg);

        mPanelGainAtten.setBorder(javax.swing.BorderFactory.createTitledBorder("Gain/Atten"));
        mPanelGainAtten.setLayout(new javax.swing.BoxLayout(mPanelGainAtten, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnIfGain.setToolTipText("Set the IF gain to setting 0=min..3=max");
        panelInstBtnIfGain.setDialogInput(""); // NOI18N
        panelInstBtnIfGain.setDoubleBuffered(false);
        panelInstBtnIfGain.setLabel("IF Gain");
        panelInstBtnIfGain.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnIfGain.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnIfGain.setOptions(new String[] {"0", "1", "2", "3"});
        panelInstBtnIfGain.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnIfGain.setValue("88");
        panelInstBtnIfGain.setValueName("si-IfGainStr");
        panelInstBtnIfGain.setVerifyInputWhenFocusTarget(false);
        mPanelGainAtten.add(panelInstBtnIfGain);

        panelInstBtnRfChannel.setToolTipText("Reserved");
        panelInstBtnRfChannel.setDialogInput(""); // NOI18N
        panelInstBtnRfChannel.setDoubleBuffered(false);
        panelInstBtnRfChannel.setLabel("RF Channel");
        panelInstBtnRfChannel.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnRfChannel.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnRfChannel.setOptions(new String[] {"0", "1", "2", "3"});
        panelInstBtnRfChannel.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnRfChannel.setValue("88");
        panelInstBtnRfChannel.setValueName("si-RfChStr");
        panelInstBtnRfChannel.setVerifyInputWhenFocusTarget(false);
        mPanelGainAtten.add(panelInstBtnRfChannel);

        panelInstBtnCoarseAtten.setToolTipText("Reserved");
        panelInstBtnCoarseAtten.setDialogInput(""); // NOI18N
        panelInstBtnCoarseAtten.setDoubleBuffered(false);
        panelInstBtnCoarseAtten.setLabel("ATNC(dB)");
        panelInstBtnCoarseAtten.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnCoarseAtten.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnCoarseAtten.setOptions(new String[] {"0", "10", "20", "30"});
        panelInstBtnCoarseAtten.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnCoarseAtten.setValue("88");
        panelInstBtnCoarseAtten.setValueName("si-AttencStr");
        panelInstBtnCoarseAtten.setVerifyInputWhenFocusTarget(false);
        mPanelGainAtten.add(panelInstBtnCoarseAtten);

        panelInstBtnFineAtten.setToolTipText("Nest");
        panelInstBtnFineAtten.setDialogInput("DialogKeyPad");
        panelInstBtnFineAtten.setDoubleBuffered(false);
        panelInstBtnFineAtten.setLabel("ATNF(dB)");
        panelInstBtnFineAtten.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnFineAtten.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnFineAtten.setName(""); // NOI18N
        panelInstBtnFineAtten.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnFineAtten.setValue("99");
        panelInstBtnFineAtten.setValueName("si-AttenfStr");
        panelInstBtnFineAtten.setVerifyInputWhenFocusTarget(false);
        mPanelGainAtten.add(panelInstBtnFineAtten);

        mPanelRightHolder.add(mPanelGainAtten);

        mPanelMemory.setBorder(javax.swing.BorderFactory.createTitledBorder("Mem"));
        mPanelMemory.setLayout(new javax.swing.BoxLayout(mPanelMemory, javax.swing.BoxLayout.Y_AXIS));

        mPanelMemShow.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelMemShow.setMaximumSize(new java.awt.Dimension(125, 60));
        mPanelMemShow.setMinimumSize(new java.awt.Dimension(125, 60));
        mPanelMemShow.setPreferredSize(new java.awt.Dimension(125, 60));
        mPanelMemShow.setLayout(new java.awt.GridLayout(2, 2));

        mCheckBoxM0.setAlignmentX(0.5F);
        mCheckBoxM0.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        mCheckBoxM0.setLabel("0");
        mCheckBoxM0.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mCheckBoxM0ActionPerformed(evt);
            }
        });
        mPanelMemShow.add(mCheckBoxM0);

        mCheckBoxM1.setAlignmentX(0.5F);
        mCheckBoxM1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        mCheckBoxM1.setLabel("1");
        mCheckBoxM1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mCheckBoxM1ActionPerformed(evt);
            }
        });
        mPanelMemShow.add(mCheckBoxM1);

        mCheckBoxM2.setAlignmentX(0.5F);
        mCheckBoxM2.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        mCheckBoxM2.setLabel("2");
        mCheckBoxM2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mCheckBoxM2ActionPerformed(evt);
            }
        });
        mPanelMemShow.add(mCheckBoxM2);

        mCheckBoxM3.setAlignmentX(0.5F);
        mCheckBoxM3.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        mCheckBoxM3.setLabel("3");
        mCheckBoxM3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mCheckBoxM3ActionPerformed(evt);
            }
        });
        mPanelMemShow.add(mCheckBoxM3);

        mPanelMemory.add(mPanelMemShow);

        mPanelMemStore.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelMemStore.setMaximumSize(new java.awt.Dimension(125, 60));
        mPanelMemStore.setLayout(new java.awt.GridLayout(2, 2));

        mButtonM0.setLabel("0");
        mButtonM0.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonM0ActionPerformed(evt);
            }
        });
        mPanelMemStore.add(mButtonM0);

        mButtonM1.setLabel("1");
        mButtonM1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonM1ActionPerformed(evt);
            }
        });
        mPanelMemStore.add(mButtonM1);

        mButtonM2.setLabel("2");
        mButtonM2.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonM2ActionPerformed(evt);
            }
        });
        mPanelMemStore.add(mButtonM2);

        mButtonM3.setLabel("3");
        mButtonM3.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonM3ActionPerformed(evt);
            }
        });
        mPanelMemStore.add(mButtonM3);

        mPanelMemory.add(mPanelMemStore);

        mPanelRightHolder.add(mPanelMemory);

        mPanelRightControls.add(mPanelRightHolder);

        mPanelUpper.add(mPanelRightControls);

        mPanelMain.add(mPanelUpper);

        mPanelLower.setBorder(javax.swing.BorderFactory.createLineBorder(new java.awt.Color(0, 0, 0)));
        mPanelLower.setLayout(new javax.swing.BoxLayout(mPanelLower, javax.swing.BoxLayout.X_AXIS));
        mPanelMain.add(mPanelLower);

        add(mPanelMain);
    }// </editor-fold>//GEN-END:initComponents

    private void mCheckBoxM0ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mCheckBoxM0ActionPerformed
        mXySpectrum.getXyDisplay().setMemVisible(0, mCheckBoxM0.isSelected() );
    }//GEN-LAST:event_mCheckBoxM0ActionPerformed

    private void mButtonM0ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonM0ActionPerformed
        mXySpectrum.getXyDisplay().setMemCopyXy(0);
    }//GEN-LAST:event_mButtonM0ActionPerformed

    private void mCheckBoxM1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mCheckBoxM1ActionPerformed
        mXySpectrum.getXyDisplay().setMemVisible(1, mCheckBoxM1.isSelected() );
    }//GEN-LAST:event_mCheckBoxM1ActionPerformed

    private void mButtonM1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonM1ActionPerformed
        mXySpectrum.getXyDisplay().setMemCopyXy(1);
    }//GEN-LAST:event_mButtonM1ActionPerformed

    private void mCheckBoxM2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mCheckBoxM2ActionPerformed
        mXySpectrum.getXyDisplay().setMemVisible(2, mCheckBoxM2.isSelected() );
    }//GEN-LAST:event_mCheckBoxM2ActionPerformed

    private void mButtonM2ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonM2ActionPerformed
        mXySpectrum.getXyDisplay().setMemCopyXy(2);
    }//GEN-LAST:event_mButtonM2ActionPerformed

    private void mCheckBoxM3ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mCheckBoxM3ActionPerformed
        mXySpectrum.getXyDisplay().setMemVisible(3, mCheckBoxM3.isSelected() );
    }//GEN-LAST:event_mCheckBoxM3ActionPerformed

    private void mButtonM3ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonM3ActionPerformed
        mXySpectrum.getXyDisplay().setMemCopyXy(3);
    }//GEN-LAST:event_mButtonM3ActionPerformed
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton mButtonM0;
    private javax.swing.JButton mButtonM1;
    private javax.swing.JButton mButtonM2;
    private javax.swing.JButton mButtonM3;
    private javax.swing.JCheckBox mCheckBoxM0;
    private javax.swing.JCheckBox mCheckBoxM1;
    private javax.swing.JCheckBox mCheckBoxM2;
    private javax.swing.JCheckBox mCheckBoxM3;
    private javax.swing.JPanel mPanelDevice;
    private javax.swing.JPanel mPanelDisplay;
    private javax.swing.JPanel mPanelDisplayControl;
    private javax.swing.JPanel mPanelGainAtten;
    private javax.swing.JPanel mPanelLeftControls;
    private javax.swing.JPanel mPanelLeftHolder;
    private javax.swing.JPanel mPanelLower;
    private javax.swing.JPanel mPanelMain;
    private javax.swing.JPanel mPanelMemShow;
    private javax.swing.JPanel mPanelMemStore;
    private javax.swing.JPanel mPanelMemory;
    private javax.swing.JPanel mPanelResolution;
    private javax.swing.JPanel mPanelRightControls;
    private javax.swing.JPanel mPanelRightHolder;
    private javax.swing.JPanel mPanelScale;
    private javax.swing.JPanel mPanelSweepCtl;
    private javax.swing.JPanel mPanelTg;
    private javax.swing.JPanel mPanelUpper;
    private Widgets.InstButton panelInstBtnAve;
    private Widgets.InstButton panelInstBtnCoarseAtten;
    private Widgets.InstButton panelInstBtnDbPerDiv;
    private Widgets.InstButton panelInstBtnFc;
    private Widgets.InstButton panelInstBtnFineAtten;
    private Widgets.InstButton panelInstBtnFspan;
    private Widgets.InstButton panelInstBtnIfGain;
    private Widgets.InstButton panelInstBtnInt;
    private Widgets.InstButton panelInstBtnMbw;
    private Widgets.InstButton panelInstBtnNfft;
    private Widgets.InstButton panelInstBtnRef;
    private Widgets.InstButton panelInstBtnRfChannel;
    private Widgets.InstButton panelInstBtnSgDevAddr;
    private Widgets.InstButton panelInstBtnSgState;
    private Widgets.InstButton panelInstBtnSgStatus;
    private Widgets.InstButton panelInstBtnSiEnv;
    private Widgets.InstButton panelInstBtnSiPeakFrac;
    private Widgets.InstButton panelInstBtnSiPeakPick;
    private Widgets.InstButton panelInstBtnTgPoints;
    // End of variables declaration//GEN-END:variables
}
