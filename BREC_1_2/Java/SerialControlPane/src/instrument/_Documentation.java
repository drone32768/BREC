/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package instrument;

/**
 * 
 * TODO
 * x1.0 Add a device text log (device and display)
 * x2.0 Add a filter box for device to attach(prj/sn or any)
 * x3.0 Add prj/sn to title
 * x4.0 Color code by prj/sn
 * 5.0 Figure out exception with device removal
 * x6.0 Add mouse control of encoder
 * x7.0 Add auto connection option and command line/startup arguments
 * 8.0 Clean up java doc of both device and application
 * x9.0 Add a disconnect button/action
 * 10.0 Figure out if we really need file/open/close/edit
 * 11.0 Add a register snap shot under menus..
 *
 * ---------------------------------------------------------------------------
 * NOTE: Command line arguments/double click
 * 
 * On windows, to control startup with command line arguments, but use
 * double click on start files, use bat files.  To avoid the residual 
 * console windows use start with javaw:
 * 
 * # The following line leaves console window (usefull for logs)
 * java -jar scp.jar 116 2 connect
 * 
 * # The following line leaves no other windows
 * start javaw -jar scp.jar 116 2 connect
 *
 * ----------------------------------------------------------------------------
 * NOTE: Build info within application
 * 1.0 Add the BuildInfo.txt file to source:
 * 
    package instrument;

    class BuildInfo{
        public static String buildDate(){
            return "@DATE@";
        }
    }
 *
 * 2.0 Add the following lines at end of NB project "build.xml" file
    <target name="-pre-compile">
        <tstamp>
            <format property="buildTime" pattern="HH:mm:ss" locale="en,UK"/>
            <format property="buildDate" pattern="dd-MM-yyyy" locale="en,UK"/>
        </tstamp>
        <copy file="src/instrument/BuildInfo.txt" toFile="src/instrument/BuildInfo.java" overwrite="true">
          <filterset>
            <filter token="DATE" value="${buildDate}:${buildTime}"/>
          </filterset>
        </copy>
    </target>
 * @author user
 */
public class _Documentation {
    
}
