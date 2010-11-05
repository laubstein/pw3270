

#ifndef UIPARSER_H_INCLUDED

	#define UIPARSER_H_INCLUDED 1

	/**
	 * Create a toplevel window based on the XML files.
	 *
	 * @param path		Folder to search for .xml files.
	 * @param app_widget	Main application widget.
	 *
	 * @return Created toplevel window.
	 *
	 */
	LOCAL_EXTERN GtkWidget * create_window_from_ui_files(const gchar *path, GtkWidget *app_widget);


	/**
	 * Setup action attributes.
	 *
	 * Set action properties based on the xml descriptor. This function will be called for every
	 * entry of the action in the xml files but, the callback will be set only in the first call.
	 *
	 * @param action	Action object to setup.
	 * @param name		Action callback name.
	 * @param connect	Connect signals.
	 * @param names		Attribute names.
	 * @param values	Atrribute values.
	 * @param error		Pointer to the error object.
	 *
	 * @return 0 if the action is valid, non zero if not
	 *
	 */
	LOCAL_EXTERN int action_setup_default(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error);

	LOCAL_EXTERN int action_setup_toggle(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error);
	LOCAL_EXTERN int action_setup_toggleset(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error);
	LOCAL_EXTERN int action_setup_togglereset(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error);
	LOCAL_EXTERN int action_setup_pfkey(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error);
	LOCAL_EXTERN int action_setup_pakey(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error);

#endif // UIPARSER_H_INCLUDED
