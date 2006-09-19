

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

   private:
      GdkFont *fn;

   };

   /**
    * Objeto representando um componente da tela.
    *
    */
   class ScreenElement
   {
   public:
      ScreenElement();

   private:
      gchar chr;

   };

   /**
    * Objeto representando um terminal.
    *
    */
   class Terminal
   {
   public:
      Terminal(int rows = 25, int cols = 80);
      ~Terminal();

      void SetContainer(GtkContainer *ctg);

   private:
      GtkWidget 		*widget;
      int 				rows;
      int 				cols;
      FontElement		*fontlist;
      ScreenElement		*screen;
   };


#endif
