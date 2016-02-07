
package Main;

/**
 * TODO:
 * 
 * 1) Fix ADC 64k limit / 32x > 2 ave
 */

/**
 *
 * The overall goal of the application design is to factor the system into
 * a set of models with any number of independent views and simple controllers.
 * The factoring of models/views/controllers is partially driven by the
 * persistence approach.  In the interest of reducing specialized code to
 * save and restore configurations, the native bean persistence support is used
 * heavily.  Some swing elements are not serializable.  This impacts the 
 * factoring and some model internals (i.e. model listeners are marked
 * as transient so they are not instantiated).
 * 
 * The basic approach is that a set of models exist for the main elements of the 
 * system (each model may or may not have/require an autonomous thread to do
 * its processing).  The models are created or instantiated before the
 * non-persistent HMI components are (to allow them to reference model state
 * on initialization if absolutely necessary).  Following this, the models
 * are configured, enabled, and property changes fired.
 * 
 * The controller factoring is chosen to produce the simplest event handling
 * possible.  The model and view are decoupled by a property change handler
 * that uses introspection to shuttle property changes back and forth between
 * the view and the model.  This allows complete independence between model 
 * and view and removes instantiation ordering issues and value synchronization
 * problems.  It does require the model to be able to re-fire all of its 
 * property changes on demand to allow views to be dynamically created or
 * created and registered in any order relative to the model configuration and
 * loading.
 * 
 * Model threading is another area with considerations.  The first of which is
 * thread starting/stopping.  Since the java persistence infrastructure is used
 * it means that model objects will be instantiated as part of the save/load
 * process (i.e. a reference object is instantiated and then used to compare
 * against the saved/loaded object to find deltas from the default constructor)
 * If care is not taken, and the constructor for a model starts a thread, then
 * there will be lots of zombie threads just due to save/load.  For this reason
 * the model constructor does nothing to use resources until an explicit start
 * method is invoked.
 * 
 * Likewise, thread coordination between model threads and swing need to be 
 * considered.  In general we try not to mix swing thread interaction in 
 * model methods with autonomous model threads.
 * 
 */
public class _Documentation {
    
}
