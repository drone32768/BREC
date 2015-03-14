/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package Devs;

/**
 *
 * @author user
 */
public interface AdcNet {
    public int GetStreamSamples( double samples[], int nSamples );
    public int Open( String IpStr, int port, int timeoutMs );
    public double GetSamplesPerSecond();
    public int Close();
}
