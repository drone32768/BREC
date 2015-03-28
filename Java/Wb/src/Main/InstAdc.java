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

import Models.AdcModel;
import Models.XyModel;
import Widgets.PropDispatcher;

/**
 *
 * This class implements an ADC monitoring instrument
 */
public class InstAdc extends javax.swing.JPanel {
    private AdcModel               mAdcModel;
    private XyModel                mXySpectrum;
    private XyModel                mXyTimeSeries;
    private PropDispatcher         mDispatcher;   
    private String                 mApplTitleStr;
    
    /**
     * Creates new form InstAdc
     */
    public InstAdc() {
                        
        // Setup the application title string
        // mApplTitleStr = "Adc "  + BuildInfo.buildDate(); 
        mApplTitleStr = "Adc";
        
        // Create a property change dispatcher
        mDispatcher = new PropDispatcher();
        
        // Create the models 
        mAdcModel     = new AdcModel();
        mXySpectrum   = new XyModel();
        mXyTimeSeries = new XyModel();
        
        // Register the adc model as the source of the xy models
        mXySpectrum.RegisterXySrc(   mAdcModel, 0 );
        mXyTimeSeries.RegisterXySrc( mAdcModel, 1 );
                
        // Create the primary xy display
        mXySpectrum.getXyDisplay().setXyCapacity(       4096);
        mXySpectrum.getXyDisplay().setMarkersVisible(   false);
        mXySpectrum.getXyDisplay().setMarkerActive(     9);
        mXySpectrum.getXyDisplay().setMarkerVisible(    9, true);
        mXySpectrum.getXyDisplay().setXlimits(          -22000, +22000);
        mXySpectrum.getXyDisplay().setXmajorTicks(      10);
        mXySpectrum.getXyDisplay().setYlimits(          -160, +10);
        mXySpectrum.getXyDisplay().setYmajorTicks(      11);
        mXySpectrum.getXyDisplay().setXlabel(          "F(Hz)");
        mXySpectrum.getXyDisplay().setYlabel(          "dB FS");
        mXySpectrum.getXyDisplay().setTitleStr(        mApplTitleStr);
        mXySpectrum.getXyDisplay().setCaptionStr(      "caption1");
        mXySpectrum.getXyDisplay().setBackground(new java.awt.Color(255, 255, 255));
        mXySpectrum.getXyDisplay().setEnvVisible(false);
      
        // Create the secondary xy display
        mXyTimeSeries.getXyDisplay().setXyCapacity(       4096);
        mXyTimeSeries.getXyDisplay().setMarkersVisible(   false);
        mXyTimeSeries.getXyDisplay().setMarkerActive(     9);
        mXyTimeSeries.getXyDisplay().setMarkerVisible(    9, true);
        mXyTimeSeries.getXyDisplay().setXlimits(          -22000, +22000);
        mXyTimeSeries.getXyDisplay().setXmajorTicks(      10);
        mXyTimeSeries.getXyDisplay().setXlabel(          "Samples");
        mXyTimeSeries.getXyDisplay().setYlabel(          "Value");
        mXyTimeSeries.getXyDisplay().setTitleStr(        mApplTitleStr);
        mXyTimeSeries.getXyDisplay().setCaptionStr(      "caption1");
        mXyTimeSeries.getXyDisplay().setBackground(new java.awt.Color(255, 255, 255));
        mXyTimeSeries.getXyDisplay().setEnvVisible(false);
        mXyTimeSeries.getXyDisplay().setYmajorTicks(11);
        mXyTimeSeries.getXyDisplay().setYlimits(0,4096); 
        
        // Initialize the generated ui components
        initComponents();
       
        // Register the  models with dispatcher
        mDispatcher.RegisterModel(mAdcModel,     "adc" );
        mDispatcher.RegisterModel(mXySpectrum,   "spec" );
        mDispatcher.RegisterModel(mXyTimeSeries, "ts" );
        
        // Register the component tree containing all buttons with dispatcher
        mDispatcher.RegisterComponentTree(mPanelSgControl);
        
        //  Establish property change linkage with model
        mAdcModel.addPropertyChangeListener(mDispatcher.GetPropListener());
        
        // Start the engines
        mAdcModel.Start();  

        // Re-fire the models to ensure hmi components match current model
        mAdcModel.FireAll();
        mXySpectrum.FireAll();
        mXyTimeSeries.FireAll();
        
        // Set display model defaults
        mXySpectrum.setYrefStr("0");
        mXySpectrum.setYdivStr("12");
        mXyTimeSeries.setYmaxStr("4096");
        mXyTimeSeries.setYminStr("0");
        
        // Limit amount of inform displayed in time series
        mXyTimeSeries.MaxPoints( 256 );

        // Add XY display reserved panel
        mPanelDisplay.add(mXySpectrum.getXyDisplay());
  
        // Add XY display reserved panel
        mPanelDisplay.add(mXyTimeSeries.getXyDisplay());
        
        // Force set the scales after the buttons have been created
        panelInstBtnRef.setValue("0");
        panelInstBtnDbPerDiv.setValue("10");
        panelInstBtnTimeYmax.setValue("16384");
        panelInstBtnTimeYmin.setValue("0");
        
        // Provide the models a UI based thread update at video refresh rate
        ActionListener taskPerformer = new ActionListener() {
    	   public void actionPerformed(ActionEvent evt) {
               mAdcModel.UiUpdateRequest();
               mXySpectrum.UiUpdateRequest();
               mXyTimeSeries.UiUpdateRequest();
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

        mPanelLeftControls = new javax.swing.JPanel();
        mPanelSgControl = new javax.swing.JPanel();
        mPanelCtl = new javax.swing.JPanel();
        panelInstBtnSgState = new Widgets.InstButton();
        panelInstBtnSgStatus = new Widgets.InstButton();
        panelInstBtnSgDevAddr = new Widgets.InstButton();
        mPanelAdcCtl = new javax.swing.JPanel();
        panelInstBtnSettleMs = new Widgets.InstButton();
        panelInstBtnSgNest = new Widgets.InstButton();
        mPanelFreqDisplayCtl = new javax.swing.JPanel();
        panelInstBtnRef = new Widgets.InstButton();
        panelInstBtnDbPerDiv = new Widgets.InstButton();
        mPanelTimeDisplayCtl = new javax.swing.JPanel();
        panelInstBtnTimeYmax = new Widgets.InstButton();
        panelInstBtnTimeYmin = new Widgets.InstButton();
        mPanelDisplay = new javax.swing.JPanel();

        setLayout(new javax.swing.BoxLayout(this, javax.swing.BoxLayout.LINE_AXIS));

        mPanelLeftControls.setAlignmentY(0.0F);
        mPanelLeftControls.setLayout(new javax.swing.BoxLayout(mPanelLeftControls, javax.swing.BoxLayout.Y_AXIS));

        mPanelSgControl.setBorder(javax.swing.BorderFactory.createTitledBorder("Instrument Control"));
        mPanelSgControl.setAlignmentX(0.0F);
        mPanelSgControl.setAlignmentY(0.0F);
        mPanelSgControl.setLayout(new javax.swing.BoxLayout(mPanelSgControl, javax.swing.BoxLayout.Y_AXIS));

        mPanelCtl.setBorder(javax.swing.BorderFactory.createTitledBorder("Control"));
        mPanelCtl.setLayout(new javax.swing.BoxLayout(mPanelCtl, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnSgState.setLabel("State");
        panelInstBtnSgState.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgState.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgState.setOptions(new String[] {"ON", "OFF"});
        panelInstBtnSgState.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgState.setValue("OFF");
        panelInstBtnSgState.setValueName("adc-StateStr");
        mPanelCtl.add(panelInstBtnSgState);

        panelInstBtnSgStatus.setLabel("Status");
        panelInstBtnSgStatus.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgStatus.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgStatus.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgStatus.setValue("OK");
        panelInstBtnSgStatus.setValueName("adc-StatusStr");
        mPanelCtl.add(panelInstBtnSgStatus);

        panelInstBtnSgDevAddr.setDialogInput("DialogString");
        panelInstBtnSgDevAddr.setLabel("Device");
        panelInstBtnSgDevAddr.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgDevAddr.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgDevAddr.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgDevAddr.setValue("192.168.0.2:8000");
        panelInstBtnSgDevAddr.setValueName("adc-DevStr");
        mPanelCtl.add(panelInstBtnSgDevAddr);

        mPanelSgControl.add(mPanelCtl);

        mPanelAdcCtl.setBorder(javax.swing.BorderFactory.createTitledBorder("Processing"));
        mPanelAdcCtl.setLayout(new javax.swing.BoxLayout(mPanelAdcCtl, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnSettleMs.setToolTipText("Reserved");
        panelInstBtnSettleMs.setDialogInput("");
        panelInstBtnSettleMs.setDoubleBuffered(false);
        panelInstBtnSettleMs.setLabel("FftSize");
        panelInstBtnSettleMs.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSettleMs.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSettleMs.setOptions(new String[] {"128", "256", "512", "1024", "2048", "4096", "8192", "16384", "32768"});
        panelInstBtnSettleMs.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSettleMs.setValue("88");
        panelInstBtnSettleMs.setValueName("adc-FftSizeStr");
        panelInstBtnSettleMs.setVerifyInputWhenFocusTarget(false);
        mPanelAdcCtl.add(panelInstBtnSettleMs);

        panelInstBtnSgNest.setToolTipText("Nest");
        panelInstBtnSgNest.setLabel("Ave");
        panelInstBtnSgNest.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgNest.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgNest.setOptions(new String[] {"1", "2", "4", "8"});
        panelInstBtnSgNest.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnSgNest.setValue("99");
        panelInstBtnSgNest.setValueName("adc-AveSizeStr");
        mPanelAdcCtl.add(panelInstBtnSgNest);

        mPanelSgControl.add(mPanelAdcCtl);

        mPanelFreqDisplayCtl.setBorder(javax.swing.BorderFactory.createTitledBorder("Freq Scale"));
        mPanelFreqDisplayCtl.setLayout(new javax.swing.BoxLayout(mPanelFreqDisplayCtl, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnRef.setToolTipText("Reserved");
        panelInstBtnRef.setDialogInput("");
        panelInstBtnRef.setLabel("Ref(dB)");
        panelInstBtnRef.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnRef.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnRef.setOptions(new String[] {"0", "-10", "-20", "-30", "-40", "-50", "-60", "-70"});
        panelInstBtnRef.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnRef.setValue("0");
        panelInstBtnRef.setValueName("spec-YrefStr");
        mPanelFreqDisplayCtl.add(panelInstBtnRef);

        panelInstBtnDbPerDiv.setToolTipText("Reserved");
        panelInstBtnDbPerDiv.setLabel("dB/Div");
        panelInstBtnDbPerDiv.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnDbPerDiv.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnDbPerDiv.setOptions(new String[] {"3", "6", "10", "15"});
        panelInstBtnDbPerDiv.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnDbPerDiv.setValue("0");
        panelInstBtnDbPerDiv.setValueName("spec-YdivStr");
        mPanelFreqDisplayCtl.add(panelInstBtnDbPerDiv);

        mPanelSgControl.add(mPanelFreqDisplayCtl);

        mPanelTimeDisplayCtl.setBorder(javax.swing.BorderFactory.createTitledBorder("Time Scale"));
        mPanelTimeDisplayCtl.setLayout(new javax.swing.BoxLayout(mPanelTimeDisplayCtl, javax.swing.BoxLayout.Y_AXIS));

        panelInstBtnTimeYmax.setToolTipText("Reserved");
        panelInstBtnTimeYmax.setDialogInput("DialogKeyPad");
        panelInstBtnTimeYmax.setLabel("Ymax");
        panelInstBtnTimeYmax.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnTimeYmax.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnTimeYmax.setOptions(new String[] {});
        panelInstBtnTimeYmax.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnTimeYmax.setValue("4096");
        panelInstBtnTimeYmax.setValueName("ts-YmaxStr");
        mPanelTimeDisplayCtl.add(panelInstBtnTimeYmax);

        panelInstBtnTimeYmin.setToolTipText("Reserved");
        panelInstBtnTimeYmin.setDialogInput("DialogKeyPad");
        panelInstBtnTimeYmin.setLabel("Ymin");
        panelInstBtnTimeYmin.setMaximumSize(new java.awt.Dimension(125, 30));
        panelInstBtnTimeYmin.setMinimumSize(new java.awt.Dimension(125, 30));
        panelInstBtnTimeYmin.setOptions(new String[] {});
        panelInstBtnTimeYmin.setPreferredSize(new java.awt.Dimension(125, 30));
        panelInstBtnTimeYmin.setValue("0");
        panelInstBtnTimeYmin.setValueName("ts-YminStr");
        mPanelTimeDisplayCtl.add(panelInstBtnTimeYmin);

        mPanelSgControl.add(mPanelTimeDisplayCtl);

        mPanelLeftControls.add(mPanelSgControl);

        add(mPanelLeftControls);

        mPanelDisplay.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelDisplay.setLayout(new java.awt.GridLayout(0, 1));
        add(mPanelDisplay);
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel mPanelAdcCtl;
    private javax.swing.JPanel mPanelCtl;
    private javax.swing.JPanel mPanelDisplay;
    private javax.swing.JPanel mPanelFreqDisplayCtl;
    private javax.swing.JPanel mPanelLeftControls;
    private javax.swing.JPanel mPanelSgControl;
    private javax.swing.JPanel mPanelTimeDisplayCtl;
    private Widgets.InstButton panelInstBtnDbPerDiv;
    private Widgets.InstButton panelInstBtnRef;
    private Widgets.InstButton panelInstBtnSettleMs;
    private Widgets.InstButton panelInstBtnSgDevAddr;
    private Widgets.InstButton panelInstBtnSgNest;
    private Widgets.InstButton panelInstBtnSgState;
    private Widgets.InstButton panelInstBtnSgStatus;
    private Widgets.InstButton panelInstBtnTimeYmax;
    private Widgets.InstButton panelInstBtnTimeYmin;
    // End of variables declaration//GEN-END:variables
}
