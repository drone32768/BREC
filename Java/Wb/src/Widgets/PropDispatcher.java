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

import java.awt.Component;
import java.awt.Container;
import java.beans.PropertyChangeListener;
import java.lang.reflect.Method;
import java.util.HashMap;

/**
 * This class implements an object that shuffles property changes between
 * instrumentation button beans and instrument model beans.
 * 
 * A model is registered with a string name.  An instrumentation widget
 * is registered with its valueName which is assumed to have the form
 * "objname"."propertyName"
 * 
 */
public class PropDispatcher {
    
    private HashMap<String, Object> nameToObj;
    private HashMap<Object, String> objToName;
    
    private PropertyChangeListener  mPropChangeListener;
    private int                     mLogLvl;
    
    public PropDispatcher(){
        mLogLvl        = 1;
        nameToObj      = new HashMap<String, Object>();
        objToName      = new HashMap<Object, String>();
        
        mPropChangeListener = new java.beans.PropertyChangeListener() {
            public void propertyChange(java.beans.PropertyChangeEvent evt) {
                DispatchPropertyChange(evt);
            }     
        };
    }
    
    public void RegisterModel( Object obj, String objName ){
        nameToObj.put( objName, obj );
        objToName.put( obj, objName );
    }
    
    public void RegisterComponentTree( Container ctree ){
        RegisterWidgets( ctree );
    }
    
    public java.beans.PropertyChangeListener GetPropListener(){
        return( mPropChangeListener );
    }
    
    public void DispatchPropertyChange(java.beans.PropertyChangeEvent evt) {  
        String objName, propName;
        
        if( mLogLvl>10 ) {
            System.out.println("DPC: prop="+evt.getPropertyName() );
        }
        
        // Split objName and propertyName
        String strs[] = evt.getPropertyName().split("-");
        
        // If the property name is not prefixed by an "objName." then
        // the source is a model.  In this case we add the objName prefix
        // and set any buttons
        if( 1==strs.length ){
            if( mLogLvl>10 ){
                System.out.println("DPC: From Model PropChange[ " +
                                    " name="+evt.getPropertyName() +
                                    " old="+evt.getOldValue()+
                                    " new="+evt.getNewValue() +
                                    " ] ");
            }
                        
            objName = objToName.get( evt.getSource() );
            if( null == objName ){
                if( mLogLvl > 0 ){
                    System.out.println("DPC:--cannot find a registered model object");
                }
                return;
            }
 
            InstWidget widget = (InstWidget)nameToObj.get( 
                                               objName+ 
                                               "-" +
                                               evt.getPropertyName() );
            if( null!=widget ){
                widget.setValue((String)evt.getNewValue());
            }
            else{
                if( mLogLvl > 0 ){
                    System.out.println("DPC:--no listening button for "+
                            evt.getPropertyName()+" newValue="+evt.getNewValue());
                }
            }
        }
        
        // The property name does contain an "objName" prefix so look up
        // the model, strip the objName from the propertyName, lookup the
        // object and use introspection to invoke the setter of propery name
        else{
            objName = strs[0];
            propName= strs[1];
            
            if(mLogLvl>10){
                System.out.println("DPC: From Widget PropChange[ " +
                    " name="+evt.getPropertyName() +
                    " old="+evt.getOldValue()+
                    " new="+evt.getNewValue() +
                    " ] ");
            }
            // Use introspection to invoke the setter method of the property name
            // on the model object.
            try {
                Object o     = nameToObj.get( objName );
                if( null==o ){
                    if(mLogLvl>0){
                        System.out.println("DPC:--cannot find object "+objName);
                    }
                    return;
                }
                String mName = "set" + propName;
                Method m     = o.getClass().getDeclaredMethod(mName, String.class );
                m.invoke( o, evt.getNewValue() );
            }
            catch( Exception e ){
                if( mLogLvl > 0 ){
                    System.out.println("DPC: Model Reflection exc:" + e );
                }
            }
        }
    }                                                           
   
    private void RegisterWidgets( Container topC ){
        Component[] components = topC.getComponents();
        
        if( mLogLvl > 10 ) {
            System.out.println("RegisterWidgets: Start");
        }
        
        for (int i = 0; i < components.length; i++)
        {
            Object obj = components[i];
            //if( obj.getClass()== javax.swing.JPanel.class ){
            if( obj instanceof javax.swing.JPanel ){
                RegisterWidgets( (Container)obj );
            }
            // if( obj.getClass()  InstWidget.class ){
            if( obj instanceof InstWidget ){
                InstWidget widget = (InstWidget)obj;
                if( mLogLvl>10 ){
                    System.out.println("   Register: "+widget.getValueName() );
                }
                widget.addPropertyChangeListener( GetPropListener() );
                RegisterModel( widget, widget.getValueName() );
            }
        }
        if( mLogLvl > 10 ){
            System.out.println("RegisterWidgets: End");
        }
    }
    
}
