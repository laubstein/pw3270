

#ifndef TERMINAL_H_INCLUDED

   #define TERMINAL_H_INCLUDED

   #include <gtk/gtk.h>
   #include <gdk/gdk.h>

//   #define SCREEN_ELEMENT gchar
//   #define FONT_ELEMENT   GdkFont

   /**
    * Objeto representando uma fonte.
    */
   class FontElement
   {
   public:
      FontElement();

      void Load(const char *descr);

      /**
	   * Obtem a largura da fonte.
	   *
	   */
	  int Width(void)
	  {
	     return width;
	  }

      /**
       * Obtem a altura da fonte.
       *
       */
	  int Height(void)
	  {
	     return height;
	  }

	  GdkFont *Font(void)
	  {
	  	return fn;
	  }

   private:
      GdkFont *fn;
      int	  width;
      int	  height;

   };

   /**
    * Objeto representando um componente da tela.
    *
    */
   class ScreenElement
   {
   public:
      ScreenElement();

      void expose(GtkWidget *widget, FontElement *font, int x, int y);

   private:
      gchar chr;

   };

   /**
    * Objeto representando um terminal.
    *
    */
   #define TERMINAL_HPAD 2
   #define TERMINAL_VPAD 2

   class Terminal
   {
   public:
      Terminal(int rows = 25, int cols = 80);
      ~Terminal();

      int  Print(int row, int col, const char *fmt, ...);

      void SetContainer(GtkContainer *ctg);

      /* Callbacks */
      gboolean expose(GtkWidget *widget, GdkEventExpose *event);
      void     resize(GtkWidget *widget, GtkAllocation  *allocation);

   private:
      GtkWidget 		*widget;
      int 				rows;
      int 				cols;
      int				left;
      int				top;
      FontElement		*fontlist;
      FontElement		*font;
      ScreenElement		*screen;

      void SetFont(FontElement *fn)
      {
	     font = fn;
      }

   };


#endif
