
 #include <stdarg.h>

 #include "log.h"
 #include "terminal.h"

/*---[ Font list ]------------------------------------------------------------*/

 static const char *FontDescr[] =
	{

		"-xos4-terminus-medium-*-normal-*-12-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-14-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-16-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-20-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-24-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-28-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-32-*-*-*-*-*-*-*",

		"-xos4-terminus-bold-*-*-*-12-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-14-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-16-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-20-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-24-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-28-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-32-*-*-*-*-*-*-*"

	};

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

 	top		   = TERMINAL_VPAD;
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
   // FIXME (perry#2#): Pintar somente a parte do texto que for realmente necessaria.

   int x;
   int y = top + font->Height();

   for(int lin = 0; lin < rows; lin++)
   {
   	  x  = left;
      y += font->Height();

      for(int col = 0; col < cols;col++)
      {
	     ScreenElement *s = screen+((lin*cols)+col);
         s->expose(widget, font, x, y);
	     x += font->Width();
      }
   }

   return TRUE;
 }

 /**
  * Imprime na tela.
  *
  * Formata e apresenta a string informada dentro da tela.
  *
  * @param	row		Linha da tela
  * @param	col		Coluna da tela
  * @param	attr	Atributo para a string
  * @param	fmt		String de formatacao.
  * @param	...		Argumentos.
  */
 int Terminal::Print(int row, int col, unsigned short attr, const char *fmt, ...)
 {
 	char		string[4096];
    va_list 	arg_ptr;
    char    	*ptr;

    va_start(arg_ptr, fmt);
    vsnprintf(string, 4095, fmt, arg_ptr);
    va_end(arg_ptr);

    for(ptr=string;*ptr && row < rows;ptr++)
    {
	   switch(*ptr)
	   {
	   // Testa por caractere de controle
	   case '\n':
	   case '\r':
	      col = 0;
	      row++;
	      break;

       // Nao e controle, coloca no buffer
	   default:

	      screen[(row*cols)+col].Set(*ptr,attr);

          if(col++ > cols)
          {
	         col = 0;
	         row++;
          }
	   }

    }

    // FIXME (perry#2#): Invalidar somente a area alterada.
    gdk_window_invalidate_rect(widget->window,&widget->allocation,0);

    return 0;
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
 	chr = ' ';
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
 	fn    = 0;
 	width = 0;
 }

 /**
  * Pinta o elemento.
  *
  * Pinta o elemento de tela na posicao especificada, respeitando os atributos
  * correspondentes.
  *
  */
 void ScreenElement::expose(GtkWidget *widget, FontElement *font, int x, int y)
 {
   gdk_draw_text(	widget->window,
                    font->Font(),
                    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                    x,
                    y,
                    &chr,
                    1);
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

    if(!fn)
    {
 	   DBGPrintf("Can't load \"%s\"",descr);
 	   return;
    }

    /* Obtem a geometria da fonte */
    gdk_text_extents(fn,"A",1,&lbearing,&rbearing,&width,&ascent,&descent);
    this->width  = width;
    this->height = (ascent+descent)+2;

 	DBGPrintf("Loading \"%s\" (width=%d,ascent=%d,descent=%d)",descr,width,ascent,descent);


 }

