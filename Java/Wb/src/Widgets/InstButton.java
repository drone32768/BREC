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
package Widgets;

import javax.swing.JFrame;
import javax.swing.SwingUtilities;

/**
 * This class implements an instrumentation button bean.  It is a subclass
 * of a swing panel object and suitable for inclusion within a swing component.
 * The basic philosophy is that of a button on an instrument in a space limited
 * but physical environment.  Each press of the button changes its value.  The
 * value of the button is displayed on it.  Each button has a text label 
 * describing the contents of the button.
 * 
 *   +-------------------------------------------------+
 *   |                       |                         |
 *   |    label              |       value             |
 *   |  (simple text)        |     (simple button)     |
 *   |                       |                         |
 *   +-------------------------------------------------+
 * 
 * An instrumentation button fires a property change every time its
 * value changes (either user input of set programmatically).  The name of
 * in the property change is the valueName setting of the bean.
 * 
 * The value of a button when its pressed are determined by the options 
 * provided or a specified dialog class name.  If the dialog class name is 
 * found a dialog of that type is used to obtain the new input from the user.
 * If no dialog class is specified, the options are consulted.  If the options
 * are null, then the button is effectively a status button and no user input
 * is accepted.  If the options are present, then the current button value is
 * looked up in the options and the value of the button is moved to the next
 * option present (first option if the value does not match any in the options).
 * 
 * To effectively use this bean with a gui designer and within an application
 * based on an MVC, you must do the following things:
 * 
 * 1.0  To view, place this bean within designer/application code:
 *       1.1 Place this component
 *       1.2 Set the "label" attribute of the component to a user meaningful
 *           text screen value (i.e. QuantityX)
 *       1.3 Set the "value" attribute of the component to anything that
 *           is representative. (i.e. 3.141)
 *       1.4 Set the "valueName" attribute to setter of the model bean that
 *           controls this quantity. (i.e. Xctl, and model will have setXctl())
 *       1.5 Set the "options" as you want this button to behave.  For
 *           status buttons, do not set (it defaults to null).  For simple
 *           toggle buttons add the two options ("ON" "OFF").
 *       1.6 For buttons require numeric input, the options do not matter,
 *           however, the dialog input should be specified as a keypad (i.e.
 *           setDialogInput( "DialogKeyPad" ), this is done on an gui 
 *           designer as setting the DialogInput property to DialogKeyPad)
 *       1.7 Edit this bean's events within the gui designer to identify
 *           the property change listener.
 * 2.0  To the model bean which controls this data:
 *       2.1 Ensure that the "valueName" of a placed button has a corresponding
 *           setter/getter (i.e. if you assign a valueName of Xctl to a button
 *           the model must have a setXctl( String ) and String getXctl()
 *           accessor).
 *       2.2 Within the model ensure that the accessor string values for a value
 *           have human readable/meaningful values since they are displayed
 *           within the button.  In addition, all string inputs should be checked
 *           since they may come from not just lists but free form user input.
 *       2.3 Make sure that the model bean fires property changes with the
 *           appropriate name (these will be used to update the gui and is 
 *           important not just for multiple views but also initialization values.
 *       2.4 Register the property change listener of the controller on the model.
 * 3.0 To the controller, implement a property change listener/
 *       3.1 Check that the listener is registered on both the model and the
 *           view (i.e. instances of this button)
 *       3.2 The property change listener within the controller should just
 *           translate property changes on values from the model to the 
 *           view and the other way.
 * 
 */
public class InstButton extends InstWidget { // extends javax.swing.JPanel  {

    /**
     * Creates new form InstButton
     */
    public InstButton() {
        initComponents();
        mOptions     = null;
        mDialogClass = "";
        mValueName   = "value";
    }
    
    /**
     * Label is the string containing descriptive text for button.
     * This is displayed as a label object.
     * 
     * @param str 
     */
    public void setLabel( String str ){
        jLabel1.setText( str );
    }
    public String getLabel(){
        return( jLabel1.getText() );
    }
    
    /**
     * ValueName is the string which contains the name of property when 
     * reporting value changes.  (i.e. on property change notifications,
     * this string will be used as the property name)
     */
    String mValueName;
    public void setValueName( String str ){
        mValueName = str;
    }
    public String getValueName(){
        return(mValueName);
    }
    
    /**
     * Value is the string which contains the text display on the button
     * AND is reported as the old/new values in any property update 
     * notifications.
     * 
     * @param str 
     */
    public void setValue( String str ){
        if( 0!=str.compareTo(jButton1.getText()) ){
            String old = jButton1.getText();
            jButton1.setText(str);
            firePropertyChange(mValueName,old,str);
            System.out.println("InstButton:property change="+mValueName+" old="+old+" new="+str); 
        }
    }
    public String getValue(){
        return( jButton1.getText() );
    }
    
    /**
     * DialogInput is a string specifying an input dialog
     * class to use when pressed.  If not set or null  the input
     * method will fall to rotating through the option strings.
     */
    String mDialogClass;
    public void setDialogInput( String str ){
        mDialogClass = str;
    }
    
    /**
     * The array of options strings specifies the possible values of the 
     * button.
     */
    String [] mOptions;
    public void setOptions( String[] options ){
        mOptions = options;
    }
    
    public String [] getOptions(){
        return(mOptions);
    }
    
    /**
     * Internal support routine to find the given string in the array of 
     * option strings and return its index.  Returns -1 of string is not
     * found.
     * @param str
     * @return 
     */
    private int FindIndexInOptions( String str ){
        int idx;
        for(idx=0;idx<mOptions.length;idx++){
            if( 0==str.compareTo(mOptions[idx]) ) {
                 return(idx);
            }
        }
        return(-1);
    }
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jLabel1 = new javax.swing.JLabel();
        jButton1 = new javax.swing.JButton();

        setBorder(javax.swing.BorderFactory.createEtchedBorder());
        setLayout(new java.awt.GridLayout(1, 0));

        jLabel1.setText("jLabel1");
        add(jLabel1);

        jButton1.setText("jButton1");
        jButton1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton1ActionPerformed(evt);
            }
        });
        add(jButton1);
    }// </editor-fold>//GEN-END:initComponents

    private void jButton1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton1ActionPerformed
        if( 0==mDialogClass.compareTo("DialogKeyPad") ){
            // TODO - need to convert this to class name lookup and use
            DialogKeyPad kp = new DialogKeyPad((JFrame)SwingUtilities.getRoot(this) ,true);
            kp.setLocationRelativeTo(this);
            kp.setTitle(jLabel1.getText());
            kp.setText( jButton1.getText() );
            kp.setVisible(true);  
            
            if( kp.getInputOk() ){
               setValue( kp.getText() );  // Use set to fire prop change and set value
            }
        }
        else if( 0==mDialogClass.compareTo("DialogString") ){
            // TODO - need to convert this to class name lookup and use
            DialogString kp = new DialogString((JFrame)SwingUtilities.getRoot(this) ,true);
            kp.setLocationRelativeTo(this);
            kp.setTitle(jLabel1.getText());
            kp.setText( jButton1.getText() );
            kp.setVisible(true);  
            
            if( kp.getInputOk() ){
               setValue( kp.getText() );  // Use set to fire prop change and set value
            }            
        }
        else if( null!=mOptions ){
            int idx;
            idx = FindIndexInOptions( jButton1.getText() );
            idx = (idx+1) % mOptions.length;
            setValue( mOptions[idx] );  // Use set to fire prop change
        }
        else{
                ; // No options, and no input specified - static msg
        }
    }//GEN-LAST:event_jButton1ActionPerformed

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton jButton1;
    private javax.swing.JLabel jLabel1;
    // End of variables declaration//GEN-END:variables
}
