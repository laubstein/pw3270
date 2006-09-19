

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

 #define FONT_COUNT (sizeof(FontDescr)/sizeof(const char *))


/*---[ Event processing ]-----------------------------------------------------*/

 static gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, Terminal *t)
 {
 	return t->expose(widget,event);
 }

 static void  resize_event_callback(GtkWidget *widget, GtkAllocation *allocation, Terminal *t)
 {
 	t->resize(widget,allocation);
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
 	left       = TERMINAL_HPAD;

    widget     = gtk_drawing_area_new();

    screen     = new ScreenElement[rows*cols];

    // Carrega fontes pre-definidas
    fontlist   = new FontElement[FONT_COUNT];
    for(unsigned int f = 0; f < (sizeof(FontDescr)/sizeof(const char *)); f++)
       fontlist[f].Load(FontDescr[f]);

	// FIXME (perry#3#): Melhorar! Pegar fonte do arquivo de configuracao.
	SetFont(fontlist);

    // FIXME (perry#6#): Calcular o menor tamanho possivel com base na lista de fonte ao inves de assumir que a primeira e a menor.
    DBGTrace(font->Width());
    gtk_widget_set_size_request(widget, (font->Width()*cols)+(TERMINAL_HPAD<<1), (font->Height()*rows)+(TERMINAL_VPAD<<1));

    // Conecta sinais do GTK
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

 gboolean Terminal::expose(GtkWidget *widget, GdkEventExpose *event)
 {
/*
   gdk_draw_arc(widget->window,
                widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                TRUE,
                0, 0, widget->allocation.width, widget->allocation.height,
                0, 64 * 360);
*/

   gdk_draw_text(	widget->window,
                    font->Font(),
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    left,
                    TERMINAL_VPAD+font->Height(),
                    "xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.xxxxxxxxx.",
                    80);

   return TRUE;
 }

 void Terminal::resize(GtkWidget *widget, GtkAllocation *allocation)
 {
 	int			width = (allocation->width-(TERMINAL_HPAD<<1)) / cols;
 	FontElement *fn   = font;

	DBGPrintf("Resize %d,%d em %d,%d (%d)", allocation->width, allocation->height,allocation->x,allocation->y,width);

    // Procura por uma fonte que atenda EXATAMENTE o tamanho necessario
    for(unsigned int f = 0; f< FONT_COUNT;f++)
    {
    	if(fontlist[f].Width() == width)
    	{
		   DBGPrintf("Fonte %d atende exatamente a largura %d",f,width);
		   SetFont(fontlist+f);
   	       left = (allocation->width - (font->Width()*cols)) >> 1;
		   return;
    	}
    }

    // Procura pela fonte mais proxima e menor que o tamanho necessario
    for(unsigned int f = 0; f < FONT_COUNT;f++)
    {
    	if(fontlist[f].Width() <= width && fontlist[f].Width() > fn->Width())
    	   fn = fontlist+f;
    }

    if(fn == font)
    {
	   // Manter a mesma fonte, centraliza o bloco de texto
	   DBGMessage("Manter a fonte");
       left = (allocation->width - (font->Width()*cols)) >> 1;
       return;
    }

	if(fn)
	{
	   DBGPrintf("Fonte com largura %d atende a largura %d",fn->Width(),width);
	   SetFont(fn);
       left = (allocation->width - (font->Width()*cols)) >> 1;
	   return;
	}

	DBGPrintf("** Nao achei fonte que atenda a largura %d",width);


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
 	width = 0;
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
    gint lbearing	= 0;
    gint rbearing	= 0;
    gint width		= 0;
    gint ascent		= 0;
    gint descent	= 0;

 	if(fn)
 	   gdk_font_unref(fn);
    fn = gdk_font_load(descr);

    /* Obtem a geometria da fonte */
    gdk_text_extents(fn,"A",1,&lbearing,&rbearing,&width,&ascent,&descent);
    this->width  = width;
    this->height = ascent+descent;

 	DBGPrintf("Loading \"%s\" (width=%d,ascent=%d,descent=%d)",descr,width,ascent,descent);


 }

