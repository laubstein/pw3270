/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
*
*/


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

      void Set(gchar chr, unsigned short attr)
      {
      	this->chr = chr;
      }

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

      int  Print(int row, int col, unsigned short attr, const char *fmt, ...);

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
