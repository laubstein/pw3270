

 #include "log.h"
 #include "terminal.h"

/*---[ Font list ]------------------------------------------------------------*/

static const char *FontDescr[] =
	{ 	"-xos4-terminus-medium-*-normal-*-12-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-14-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-16-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-20-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-24-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-28-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-32-*-*-*-*-*-*-*"  };

/*---[ Event processing ]-----------------------------------------------------*/

 static gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, Terminal *t)
 {
   gdk_draw_arc(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                TRUE,
                0, 0, widget->allocation.width, widget->allocation.height,
                0, 64 * 360);

   return TRUE;
 }

 static void  resize_event_callback(GtkWidget *widget, GtkAllocation *allocation, Terminal *t)
 {
	DBGPrintf("Resize %d,%d", allocation->width, allocation->height);

 }

/*---[ Implement ]------------------------------------------------------------*/

 /**
  * Constroi um terminal do tamanho informado.
  *
  * Constroi um terminal com o numero de linhas e colunas informado, cria
  * o widget correspondente.
  *
  * @param	rows	Numero de linhas para o terminal.
  * @param	cols	Numero de colunas por linha.
  *
  */
 Terminal::Terminal(int rows, int cols)
 {
 	this->rows = rows;
 	this->cols = cols;

    widget     = gtk_drawing_area_new();

    screen     = new ScreenElement[rows*cols];

    // Carrega fontes pre-definidas
    fontlist   = new FontElement[(sizeof(FontDescr)/sizeof(const char *))];
    for(unsigned int f = 0; f < (sizeof(FontDescr)/sizeof(const char *)); f++)
       fontlist[f].Load(FontDescr[f]);

	// FIXME (perry#3#): Calcular o tamanho de acordo com a fonte atual.
    gtk_widget_set_size_request(widget, 100, 100);

    g_signal_connect(G_OBJECT(widget), "expose_event",  G_CALLBACK(expose_event_callback), this);
    g_signal_connect(G_OBJECT(widget), "size-allocate", G_CALLBACK(resize_event_callback), this);

 }

 Terminal::~Terminal()
 {
    if(screen)
       delete[] screen;

	if(fontlist)
	   delete[] fontlist;
 }

 void Terminal::SetContainer(GtkContainer *ctg)
 {
 	gtk_widget_show(widget);
 	gtk_container_add(ctg,widget);
 }


 /**
  * Constroi um elemento de tela.
  *
  * Constroi objeto correspondente a um elemento de tela dentro do terminal;
  * cada terminal criado mantem um vetor de objetos deste tipo cada um deles
  * contendo o caractere e atributos correspondentes.
  *
  */
 ScreenElement::ScreenElement()
 {
 	chr = 0;
 }

 /**
  * Constroi um descritor de fonte sem associacao.
  *
  * Constroi um descritor de fonte em branco, sem associar a nenhuma fonte
  * real.
  *
  */
 FontElement::FontElement()
 {
 	fn = 0;
 }

 /**
  * Carrega fonte pedida.
  *
  * Carrega a fonte cuja descricao foi informada criando os recursos gdk
  * necessarios.
  *
  * @param	descr	Descricao da fonte a ser carregada.
  *
  */
 void FontElement::Load(const char *descr)
 {
 	DBGPrintf("Loading \"%s\"",descr);

 	if(fn)
 	   gdk_font_unref(fn);
    fn = gdk_font_load(descr);
 }
