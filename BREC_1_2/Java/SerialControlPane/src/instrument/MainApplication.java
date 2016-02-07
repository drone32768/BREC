/*
 */
package instrument;

import java.io.Serializable;

import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseWheelListener;
import java.awt.event.MouseWheelEvent;

import javax.swing.SwingUtilities;
import javax.swing.JFrame;
import javax.swing.Timer;

/**
 *
 * @author jkleine
 */
public class MainApplication extends javax.swing.JFrame 
         implements Serializable, MouseListener, MouseWheelListener {

    static String [] mApplArgv;
    private PicCdc   mPic;
    
    String           mLine1Str;
    String           mLine2Str;
    boolean          mOpenRequest;
    boolean          mCloseRequest;
            
    public MainApplication(){
          // Create the rest of the application
        InitMainApplication();    
    }
  
    /**
     * Creates new form MainApplication
     */
    private void InitMainApplication() {
        
        // Initialize hmi components
        initComponents();
        
        // Register for all mouse wheel events
        this.addMouseWheelListener(this);
        this.addMouseListener(this);
        
        // Set the button prj and sn based on command line arguments
        if( mApplArgv.length >= 2 ){
            mButtonPrjId.setText( mApplArgv[0] );
            mButtonSnId.setText(  mApplArgv[1] );
            mOpenRequest = false;
        }
        else{
            mButtonPrjId.setText( "0" );
            mButtonSnId.setText(  "0" );
            mOpenRequest = false;
        }
        if( mApplArgv.length >=3 ){
            mOpenRequest = true;
        }
        mCloseRequest = false;
        
        // Set the frame title
        String applTitleStr = "SerialConsole "  + BuildInfo.buildDate(); 
        setTitle(applTitleStr);

        // Initialize the LCD lines
        mLine1Str = new String("0123456789");
        mLine2Str = new String("0123456789");
        
        // Create a pic device object
        mPic = new PicCdc();
        
        // We use a timer to get information from the device and
        // paint the relevant portions of the display. 
        ActionListener taskPerformer = new ActionListener() {
           int mIdx=0; // Rotating index of last lcd index update request
           
    	   public void actionPerformed(ActionEvent evt) {
               char [] lnch = new char[16];
                           
               // If there is an outstanding open request do it
               if( mOpenRequest ){
                   InitiateOpen();
                   mOpenRequest = false;
               }
               
               // If there is an outstanding close request do it
               if( mCloseRequest ){
                   mPic.Close();
                   mCloseRequest = false;
               }
               
               // Update the title with currently open device
               UpdateTitleString();

               // Set the current status
               mLabelStatus.setText( mPic.GetStatusStr() );
               
               // Set the current state
               mLabelState.setText( mPic.GetStateStr() );
               
               // If the device is not in the open state change
               // the foreground color to alert
               if( !mPic.IsOpen() ){
                   mLabelLcdLine1.setForeground(Color.red);
                   mLabelLcdLine2.setForeground(Color.red);
                   return;
               }  
        
               // Update the w/r byte counts
               mLabelWriteBytes.setText( Integer.toString( mPic.GetWriteBytes() ) );
               mLabelReadBytes.setText(  Integer.toString( mPic.GetReadBytes()  ) );
               
               // Get a copy of line 1
               for( int idx=0; idx<16; idx++ ){
                   lnch[idx] = (char)mPic.GetReg(0x80 + idx);
               }
               mLine1Str = new String( lnch );
               
               // Get a copy of line 2
               for( int idx=0; idx<16; idx++ ){
                   lnch[idx] = (char)mPic.GetReg(0x80 + 16 + idx);
               }
               mLine2Str = new String( lnch );

               // Update the 2 line LCD
               mLabelLcdLine1.setForeground(Color.black);
               mLabelLcdLine2.setForeground(Color.black);
               mLabelLcdLine1.setText(mLine1Str);
               mLabelLcdLine2.setText(mLine2Str);
               
               // Request a read of the lcd memory for next time
               for( int count=0; count<16; count++ ){
                   mIdx=(mIdx+1)%32;
                   mPic.Rd8( 0x80 + mIdx );
               }
    	   } // End of timer action
        };
        Timer timer = new Timer( 30 , taskPerformer);
        timer.start();
    }

    private int mLastPrjId; // prj last time title was updated
    private int mLastSnId;  // sn last time title was updated
    private void UpdateTitleString(){
        if( (mLastPrjId != mPic.GetPrjNumber()     ) || 
            (mLastSnId  != mPic.GetSerialNumber()) ){      
               String applTitleStr = "PRJ:" + mPic.GetPrjNumber()+
                                     " SN:" + mPic.GetSerialNumber()+
                                     " SerialConsole "  + BuildInfo.buildDate(); 
               setTitle(applTitleStr);
               mLastPrjId = mPic.GetPrjNumber();
               mLastSnId  = mPic.GetSerialNumber();
        }         
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
        mPanelLcd = new javax.swing.JPanel();
        mLabelLcdLine1 = new javax.swing.JLabel();
        mLabelLcdLine2 = new javax.swing.JLabel();
        mPanelEncoder = new javax.swing.JPanel();
        mButtonLeft = new javax.swing.JButton();
        mButtonSwitch = new javax.swing.JButton();
        mButtonRight = new javax.swing.JButton();
        mPanelStateAndStatus = new javax.swing.JPanel();
        mPanelState = new javax.swing.JPanel();
        mLabelState = new javax.swing.JLabel();
        mPanelStatus = new javax.swing.JPanel();
        mLabelStatus = new javax.swing.JLabel();
        mPanelWriteBytes = new javax.swing.JPanel();
        mLabelWrite = new javax.swing.JLabel();
        mLabelWriteBytes = new javax.swing.JLabel();
        mPanelReadBytes = new javax.swing.JPanel();
        mLabelRead = new javax.swing.JLabel();
        mLabelReadBytes = new javax.swing.JLabel();
        mPanelConnectControl = new javax.swing.JPanel();
        mPanelPrjId1 = new javax.swing.JPanel();
        jLabel5 = new javax.swing.JLabel();
        mButtonPrjId = new javax.swing.JButton();
        mPanelSnId1 = new javax.swing.JPanel();
        jLabel4 = new javax.swing.JLabel();
        mButtonSnId = new javax.swing.JButton();
        mButtonConnect = new javax.swing.JButton();
        mButtonDisconnect = new javax.swing.JButton();
        menuBar = new javax.swing.JMenuBar();
        fileMenu = new javax.swing.JMenu();
        openMenuItem = new javax.swing.JMenuItem();
        saveMenuItem = new javax.swing.JMenuItem();
        saveAsMenuItem = new javax.swing.JMenuItem();
        exitMenuItem = new javax.swing.JMenuItem();
        editMenu = new javax.swing.JMenu();
        cutMenuItem = new javax.swing.JMenuItem();
        copyMenuItem = new javax.swing.JMenuItem();
        pasteMenuItem = new javax.swing.JMenuItem();
        deleteMenuItem = new javax.swing.JMenuItem();
        helpMenu = new javax.swing.JMenu();
        contentsMenuItem = new javax.swing.JMenuItem();
        aboutMenuItem = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle("frame title");

        mPanelMain.setAlignmentX(0.5F);
        mPanelMain.setLayout(new javax.swing.BoxLayout(mPanelMain, javax.swing.BoxLayout.PAGE_AXIS));

        mPanelLcd.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelLcd.setAlignmentX(0.5F);
        mPanelLcd.setAlignmentY(0.0F);
        mPanelLcd.setLayout(new javax.swing.BoxLayout(mPanelLcd, javax.swing.BoxLayout.Y_AXIS));

        mLabelLcdLine1.setFont(new java.awt.Font("Courier New", 0, 48)); // NOI18N
        mLabelLcdLine1.setText("9876543210123456");
        mPanelLcd.add(mLabelLcdLine1);

        mLabelLcdLine2.setFont(new java.awt.Font("Courier New", 0, 48)); // NOI18N
        mLabelLcdLine2.setText("9876543210123456");
        mPanelLcd.add(mLabelLcdLine2);

        mPanelMain.add(mPanelLcd);

        mPanelEncoder.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelEncoder.setAlignmentY(0.0F);
        mPanelEncoder.setLayout(new java.awt.GridLayout(1, 0));

        mButtonLeft.setText("<<");
        mButtonLeft.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonLeftActionPerformed(evt);
            }
        });
        mPanelEncoder.add(mButtonLeft);

        mButtonSwitch.setText("*");
        mButtonSwitch.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonSwitchActionPerformed(evt);
            }
        });
        mPanelEncoder.add(mButtonSwitch);

        mButtonRight.setText(">>");
        mButtonRight.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonRightActionPerformed(evt);
            }
        });
        mPanelEncoder.add(mButtonRight);

        mPanelMain.add(mPanelEncoder);

        mPanelStateAndStatus.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelStateAndStatus.setAlignmentY(0.0F);
        mPanelStateAndStatus.setLayout(new java.awt.GridLayout(1, 0));

        mPanelState.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelState.setLayout(new javax.swing.BoxLayout(mPanelState, javax.swing.BoxLayout.LINE_AXIS));

        mLabelState.setText("Idle");
        mPanelState.add(mLabelState);

        mPanelStateAndStatus.add(mPanelState);

        mPanelStatus.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelStatus.setLayout(new javax.swing.BoxLayout(mPanelStatus, javax.swing.BoxLayout.LINE_AXIS));

        mLabelStatus.setText("No Status");
        mPanelStatus.add(mLabelStatus);

        mPanelStateAndStatus.add(mPanelStatus);

        mPanelWriteBytes.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelWriteBytes.setLayout(new javax.swing.BoxLayout(mPanelWriteBytes, javax.swing.BoxLayout.LINE_AXIS));

        mLabelWrite.setText("W:");
        mPanelWriteBytes.add(mLabelWrite);

        mLabelWriteBytes.setText("0");
        mPanelWriteBytes.add(mLabelWriteBytes);

        mPanelStateAndStatus.add(mPanelWriteBytes);

        mPanelReadBytes.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelReadBytes.setLayout(new javax.swing.BoxLayout(mPanelReadBytes, javax.swing.BoxLayout.LINE_AXIS));

        mLabelRead.setText("R:");
        mPanelReadBytes.add(mLabelRead);

        mLabelReadBytes.setText("0");
        mPanelReadBytes.add(mLabelReadBytes);

        mPanelStateAndStatus.add(mPanelReadBytes);

        mPanelMain.add(mPanelStateAndStatus);

        mPanelConnectControl.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelConnectControl.setAlignmentY(0.0F);
        mPanelConnectControl.setLayout(new java.awt.GridLayout(1, 0));

        mPanelPrjId1.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelPrjId1.setLayout(new javax.swing.BoxLayout(mPanelPrjId1, javax.swing.BoxLayout.LINE_AXIS));

        jLabel5.setText("PRJ");
        mPanelPrjId1.add(jLabel5);

        mButtonPrjId.setText("0");
        mButtonPrjId.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonPrjIdActionPerformed(evt);
            }
        });
        mPanelPrjId1.add(mButtonPrjId);

        mPanelConnectControl.add(mPanelPrjId1);

        mPanelSnId1.setBorder(javax.swing.BorderFactory.createEtchedBorder());
        mPanelSnId1.setLayout(new javax.swing.BoxLayout(mPanelSnId1, javax.swing.BoxLayout.LINE_AXIS));

        jLabel4.setText("SN");
        mPanelSnId1.add(jLabel4);

        mButtonSnId.setText("0");
        mButtonSnId.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonSnIdActionPerformed(evt);
            }
        });
        mPanelSnId1.add(mButtonSnId);

        mPanelConnectControl.add(mPanelSnId1);

        mButtonConnect.setText("Connect");
        mButtonConnect.setAlignmentX(0.5F);
        mButtonConnect.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonConnectActionPerformed(evt);
            }
        });
        mPanelConnectControl.add(mButtonConnect);

        mButtonDisconnect.setText("Disconnect");
        mButtonDisconnect.setAlignmentX(0.5F);
        mButtonDisconnect.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                mButtonDisconnectActionPerformed(evt);
            }
        });
        mPanelConnectControl.add(mButtonDisconnect);

        mPanelMain.add(mPanelConnectControl);

        fileMenu.setMnemonic('f');
        fileMenu.setText("File");

        openMenuItem.setMnemonic('o');
        openMenuItem.setText("Open");
        openMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                openMenuItemActionPerformed(evt);
            }
        });
        fileMenu.add(openMenuItem);

        saveMenuItem.setMnemonic('s');
        saveMenuItem.setText("Save");
        saveMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveMenuItemActionPerformed(evt);
            }
        });
        fileMenu.add(saveMenuItem);

        saveAsMenuItem.setMnemonic('a');
        saveAsMenuItem.setText("Save As ...");
        saveAsMenuItem.setDisplayedMnemonicIndex(5);
        saveAsMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveAsMenuItemActionPerformed(evt);
            }
        });
        fileMenu.add(saveAsMenuItem);

        exitMenuItem.setMnemonic('x');
        exitMenuItem.setText("Exit");
        exitMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                exitMenuItemActionPerformed(evt);
            }
        });
        fileMenu.add(exitMenuItem);

        menuBar.add(fileMenu);

        editMenu.setMnemonic('e');
        editMenu.setText("Edit");

        cutMenuItem.setMnemonic('t');
        cutMenuItem.setText("Cut");
        editMenu.add(cutMenuItem);

        copyMenuItem.setMnemonic('y');
        copyMenuItem.setText("Copy");
        editMenu.add(copyMenuItem);

        pasteMenuItem.setMnemonic('p');
        pasteMenuItem.setText("Paste");
        editMenu.add(pasteMenuItem);

        deleteMenuItem.setMnemonic('d');
        deleteMenuItem.setText("Delete");
        editMenu.add(deleteMenuItem);

        menuBar.add(editMenu);

        helpMenu.setMnemonic('h');
        helpMenu.setText("Help");

        contentsMenuItem.setMnemonic('c');
        contentsMenuItem.setText("Contents");
        helpMenu.add(contentsMenuItem);

        aboutMenuItem.setMnemonic('a');
        aboutMenuItem.setText("About");
        aboutMenuItem.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                aboutMenuItemActionPerformed(evt);
            }
        });
        helpMenu.add(aboutMenuItem);

        menuBar.add(helpMenu);

        setJMenuBar(menuBar);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(mPanelMain, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(mPanelMain, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void exitMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_exitMenuItemActionPerformed
        System.exit(0);
    }//GEN-LAST:event_exitMenuItemActionPerformed

    private void SaPropertyChangeHandler(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_SaPropertyChangeHandler

    }//GEN-LAST:event_SaPropertyChangeHandler

    private void SgPropertyChangeHandler(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_SgPropertyChangeHandler
    }//GEN-LAST:event_SgPropertyChangeHandler

    private void saveMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveMenuItemActionPerformed
        System.out.println("Save Menu Item");
    }//GEN-LAST:event_saveMenuItemActionPerformed

    private void openMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_openMenuItemActionPerformed
        System.out.println("Open Menu Item");
    }//GEN-LAST:event_openMenuItemActionPerformed

    
    private void saveAsMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveAsMenuItemActionPerformed
    }//GEN-LAST:event_saveAsMenuItemActionPerformed

    private void panelInstBtnSaCenterFSaPropertyChangeHandler(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_panelInstBtnSaCenterFSaPropertyChangeHandler
        // TODO add your handling code here:
    }//GEN-LAST:event_panelInstBtnSaCenterFSaPropertyChangeHandler

    private void panelInstBtnSaSpanFSaPropertyChangeHandler(java.beans.PropertyChangeEvent evt) {//GEN-FIRST:event_panelInstBtnSaSpanFSaPropertyChangeHandler
        // TODO add your handling code here:
    }//GEN-LAST:event_panelInstBtnSaSpanFSaPropertyChangeHandler

    private void mButtonLeftActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonLeftActionPerformed
        mPic.Wr8(0xC4, 0x0);
    }//GEN-LAST:event_mButtonLeftActionPerformed

    private void mButtonSwitchActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonSwitchActionPerformed
        mPic.Wr8(0xC0, 0x0);
        mPic.Wr8(0xC1, 0x0);
    }//GEN-LAST:event_mButtonSwitchActionPerformed

    private void mButtonRightActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonRightActionPerformed
        mPic.Wr8(0xC3, 0x0);
    }//GEN-LAST:event_mButtonRightActionPerformed

    private void aboutMenuItemActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_aboutMenuItemActionPerformed
    }//GEN-LAST:event_aboutMenuItemActionPerformed

    private void InitiateOpen()
    {
        String PrjStr = mButtonPrjId.getText();
        String SnStr  = mButtonSnId.getText();
        int    prjId  = -1;
        int    snId   = -1;
        boolean match;
        
        mPic.Close();

        if( 0==PrjStr.compareTo("0") ){
            match = false;
        }
        else{
            prjId = Integer.parseInt(PrjStr);
            snId  = Integer.parseInt(SnStr);
            match = true;
        }
        
        mPic = new PicCdc();
        mPic.OpenAsynch(match, prjId, snId);
    }

    private void mButtonConnectActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonConnectActionPerformed
        mOpenRequest = true;
    }//GEN-LAST:event_mButtonConnectActionPerformed

    private void mButtonPrjIdActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonPrjIdActionPerformed
            DialogKeyPad kp = new DialogKeyPad((JFrame)SwingUtilities.getRoot(this) ,true);
            kp.setLocationRelativeTo(this);
            kp.setText( mButtonPrjId.getText() );
            kp.setVisible(true);  
            
            if( kp.getInputOk() ){
               mButtonPrjId.setText(kp.getText() ); 
            }
    }//GEN-LAST:event_mButtonPrjIdActionPerformed

    private void mButtonSnIdActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonSnIdActionPerformed
            DialogKeyPad kp = new DialogKeyPad((JFrame)SwingUtilities.getRoot(this) ,true);
            kp.setLocationRelativeTo(this);
            kp.setText( mButtonSnId.getText() );
            kp.setVisible(true);  
            
            if( kp.getInputOk() ){
               mButtonSnId.setText(kp.getText() ); 
            }
    }//GEN-LAST:event_mButtonSnIdActionPerformed

    private void mButtonDisconnectActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_mButtonDisconnectActionPerformed
        mCloseRequest = true;
    }//GEN-LAST:event_mButtonDisconnectActionPerformed

    @Override
    public void mouseWheelMoved(MouseWheelEvent e){
        // System.out.println("Mouse wheel"+e);
        if( e.getWheelRotation() < 0 ){
            mPic.Wr8(0xC3, 0x0);    
        }
        else{
            mPic.Wr8(0xC4, 0x0);    
        }  
    }

    @Override
    public void mousePressed(MouseEvent e) {
        // Necessary for interface.  Event not used.
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        // System.out.println("Mouse res " + e );
        if( e.getButton()==2 ){
            mPic.Wr8(0xC0, 0x0);
            mPic.Wr8(0xC1, 0x0);
        }
    }

    @Override
    public void mouseEntered(MouseEvent e) {
        // Necessary for interface.  Event not used.    
    }

    @Override
    public void mouseExited(MouseEvent e) {
        // Necessary for interface.  Event not used.   
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        // Necessary for interface.  Event not used.
    }

    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        /* Set the Nimbus look and feel */
        //<editor-fold defaultstate="collapsed" desc=" Look and feel setting code (optional) ">
        /* If Nimbus (introduced in Java SE 6) is not available, stay with the default look and feel.
         * For details see http://download.oracle.com/javase/tutorial/uiswing/lookandfeel/plaf.html 
         */
        try {
            for (javax.swing.UIManager.LookAndFeelInfo info : javax.swing.UIManager.getInstalledLookAndFeels()) {
                if ("Nimbus".equals(info.getName())) {
                    javax.swing.UIManager.setLookAndFeel(info.getClassName());
                    break;
                }
            }
        } catch (ClassNotFoundException ex) {
            java.util.logging.Logger.getLogger(MainApplication.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (InstantiationException ex) {
            java.util.logging.Logger.getLogger(MainApplication.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (IllegalAccessException ex) {
            java.util.logging.Logger.getLogger(MainApplication.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        } catch (javax.swing.UnsupportedLookAndFeelException ex) {
            java.util.logging.Logger.getLogger(MainApplication.class.getName()).log(java.util.logging.Level.SEVERE, null, ex);
        }
        //</editor-fold>

        // Save a copy of command line arguments for later processing
        mApplArgv=args;
        
        /* Create and display the form */
        java.awt.EventQueue.invokeLater(new Runnable() {
            @Override
            public void run() {
                new MainApplication().setVisible(true);
            }
        });
    }
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JMenuItem aboutMenuItem;
    private javax.swing.JMenuItem contentsMenuItem;
    private javax.swing.JMenuItem copyMenuItem;
    private javax.swing.JMenuItem cutMenuItem;
    private javax.swing.JMenuItem deleteMenuItem;
    private javax.swing.JMenu editMenu;
    private javax.swing.JMenuItem exitMenuItem;
    private javax.swing.JMenu fileMenu;
    private javax.swing.JMenu helpMenu;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JButton mButtonConnect;
    private javax.swing.JButton mButtonDisconnect;
    private javax.swing.JButton mButtonLeft;
    private javax.swing.JButton mButtonPrjId;
    private javax.swing.JButton mButtonRight;
    private javax.swing.JButton mButtonSnId;
    private javax.swing.JButton mButtonSwitch;
    private javax.swing.JLabel mLabelLcdLine1;
    private javax.swing.JLabel mLabelLcdLine2;
    private javax.swing.JLabel mLabelRead;
    private javax.swing.JLabel mLabelReadBytes;
    private javax.swing.JLabel mLabelState;
    private javax.swing.JLabel mLabelStatus;
    private javax.swing.JLabel mLabelWrite;
    private javax.swing.JLabel mLabelWriteBytes;
    private javax.swing.JPanel mPanelConnectControl;
    private javax.swing.JPanel mPanelEncoder;
    private javax.swing.JPanel mPanelLcd;
    private javax.swing.JPanel mPanelMain;
    private javax.swing.JPanel mPanelPrjId1;
    private javax.swing.JPanel mPanelReadBytes;
    private javax.swing.JPanel mPanelSnId1;
    private javax.swing.JPanel mPanelState;
    private javax.swing.JPanel mPanelStateAndStatus;
    private javax.swing.JPanel mPanelStatus;
    private javax.swing.JPanel mPanelWriteBytes;
    private javax.swing.JMenuBar menuBar;
    private javax.swing.JMenuItem openMenuItem;
    private javax.swing.JMenuItem pasteMenuItem;
    private javax.swing.JMenuItem saveAsMenuItem;
    private javax.swing.JMenuItem saveMenuItem;
    // End of variables declaration//GEN-END:variables
}
