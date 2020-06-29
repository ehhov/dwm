/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int gappx     = 5;        /* gap pixel between windows */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int showsystray        = 1;        /* 0 means no systray */
static const int focusonwheel       = 0;
static const char *fonts[]          = { "monospace:size=10" };
static const char dmenufont[]       = "monospace:size=10";
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, 0: display systray on the last monitor*/
static const char col_gray1[]            = "#222222";
static const char col_gray2[]            = "#444444";
static const char col_gray3[]            = "#bbbbbb";
static const char col_gray4[]            = "#eeeeee";
static const char col_cyan[]             = "#005577";
static const char *colors[][3]           = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

/* Custom functions */
static void tile_gaps(Monitor *m);
static void tile_vert_gaps(Monitor *m);
static void tile_vert(Monitor *m);
static void tile_deck_gaps(Monitor *m);
static void tile_deck(Monitor *m);
static void monocle_gaps(Monitor *m);
static void focusurgent(const Arg *arg);
static void movestack(const Arg *arg);
static void resetlayout(const Arg *arg);
static void togglefullscreen(const Arg *arg);

/* autostart commands */
static const char *const autostart[] = {
	"sh", "-c", "xrdb -load ~/.Xresources", NULL,
	"sh", "-c", "feh --bg-fill ~/.wallpapers/foggy-forest-light.jpg", NULL,
	"sh", "-c", "~/.bin/kb", NULL,
	"dwms", NULL,
	"clipmenud", NULL,
	"dunst", NULL,
	"picom", NULL,
	NULL /* terminate */
};

/* tagging */
static const char *tags[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
	{ "Firefox",  NULL,       NULL,       1 << 8,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[T]",      tile_gaps },
	{ "[T]",      tile },
	{ "[M]",      monocle_gaps },
	{ "[M]",      monocle },
	{ "[F]",      NULL },
	{ "[V]",      tile_vert_gaps },
	{ "[V]",      tile_vert },
	{ "[D]",      tile_deck_gaps },
	{ "[D]",      tile_deck },
};

/* key definitions */
#define MODKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", NULL };

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY|ShiftMask,             XK_t,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY|ShiftMask,             XK_m,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[4]} },
	{ MODKEY,                       XK_v,      setlayout,      {.v = &layouts[5]} },
	{ MODKEY|ShiftMask,             XK_v,      setlayout,      {.v = &layouts[6]} },
	{ MODKEY,                       XK_a,      setlayout,      {.v = &layouts[7]} },
	{ MODKEY|ShiftMask,             XK_a,      setlayout,      {.v = &layouts[8]} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscreen,{0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

/* Custom functions */
void
tile_gaps(Monitor *m)
{
	unsigned int i, n, h, mw, my, ty, ns;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;

	if (n > m->nmaster) {
		mw = m->nmaster ? m->ww * m->mfact : 0;
		ns = m->nmaster > 0 ? 2 : 1;
	} else {
		mw = m->ww;
		ns = 1;
	}
	for (i = 0, my = ty = gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) - gappx;
			resize(c, m->wx + gappx, m->wy + my, mw - (2*c->bw) - gappx*(5-ns)/2, h - (2*c->bw), 0);
			if (my + HEIGHT(c) + gappx < m->wh)
				my += HEIGHT(c) + gappx;
		} else {
			h = (m->wh - ty) / (n - i) - gappx;
			resize(c, m->wx + mw + gappx/ns, m->wy + ty, m->ww - mw - (2*c->bw) - gappx*(5-ns)/2, h - (2*c->bw), 0);
			if (ty + HEIGHT(c) + gappx < m->wh)
				ty += HEIGHT(c) + gappx;
		}
}

void
tile_vert_gaps(Monitor *m)
{
	unsigned int i, n, w, mh, mx, tx, ns;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;

	if (n > m->nmaster) {
		mh = m->nmaster ? m->wh * m->mfact : 0;
		ns = m->nmaster > 0 ? 2 : 1;
	} else {
		mh = m->wh;
		ns = 1;
	}
	for (i = 0, mx = tx = gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			w = (m->ww - mx) / (MIN(n, m->nmaster) - i) - gappx;
			resize(c, m->wx + mx, m->wy + gappx, w - (2*c->bw), mh - (2*c->bw) - gappx*(5-ns)/2, 0);
			if (mx + WIDTH(c) + gappx < m->ww)
				mx += WIDTH(c) + gappx;
		} else {
			w = (m->ww - tx) / (n - i) - gappx;
			resize(c, m->wx + tx, m->wy + mh + gappx/ns, w - (2*c->bw), m->wh - mh - (2*c->bw) - gappx*(5-ns)/2, 0);
			if (tx + WIDTH(c) + gappx < m->ww)
				tx += WIDTH(c) + gappx;
		}
}

void
tile_vert(Monitor *m)
{
	unsigned int i, n, w, mh, mx, tx;
	Client *c;

	for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if (n == 0)
		return;

	if (n > m->nmaster)
		mh = m->nmaster ? m->wh * m->mfact : 0;
	else
		mh = m->wh;
	for (i = 0, mx = tx = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if (i < m->nmaster) {
			w = (m->ww - mx) / (MIN(n, m->nmaster) - i);
			resize(c, m->wx + mx, m->wy, w - (2*c->bw), mh - (2*c->bw), 0);
			if (mx + WIDTH(c) < m->ww)
				mx += WIDTH(c);
		} else {
			w = (m->ww - tx) / (n - i);
			resize(c, m->wx + tx, m->wy + mh, w - (2*c->bw), m->wh - mh - (2*c->bw), 0);
			if (tx + WIDTH(c) < m->ww)
				tx += WIDTH(c);
		}
}

void
tile_deck_gaps(Monitor *m)
{
	unsigned int i, n, h, mw, my, ns;
	Client *c;

	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;

	if(n > m->nmaster) {
		mw = m->nmaster ? m->ww * m->mfact : 0;
		ns = m->nmaster > 0 ? 2 : 1;
		if (n > m->nmaster) snprintf(m->ltsymbol, sizeof(m->ltsymbol), "[%d]", n - m->nmaster);
	}
	else {
		mw = m->ww;
		ns = 1;
	}
	for(i = 0, my = gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if(i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i) - gappx;
			resize(c, m->wx + gappx, m->wy + my, mw - (2*c->bw) - gappx*(5-ns)/2, h - (2*c->bw), 0);
			if (my + HEIGHT(c) + gappx < m->wh)
				my += HEIGHT(c) + gappx;
		}
		else
			resize(c, m->wx + mw + gappx/ns, m->wy + gappx, m->ww - mw - (2*c->bw) - gappx*(5-ns)/2, m->wh - (2*c->bw) - 2*gappx, 0);
}

void
tile_deck(Monitor *m)
{
	unsigned int i, n, h, mw, my;
	Client *c;

	for(n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++);
	if(n == 0)
		return;

	if(n > m->nmaster) {
		mw = m->nmaster ? m->ww * m->mfact : 0;
		if (n > m->nmaster) snprintf(m->ltsymbol, sizeof(m->ltsymbol), "[%d]", n - m->nmaster);
	}
	else
		mw = m->ww;
	for(i = my = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++)
		if(i < m->nmaster) {
			h = (m->wh - my) / (MIN(n, m->nmaster) - i);
			resize(c, m->wx, m->wy + my, mw - (2*c->bw), h - (2*c->bw), 0);
			if (my + HEIGHT(c) < m->wh)
				my += HEIGHT(c);
		}
		else
			resize(c, m->wx + mw, m->wy, m->ww - mw - (2*c->bw), m->wh - (2*c->bw), 0);
}

void
monocle_gaps(Monitor *m)
{
	unsigned int n = 0;
	unsigned int gap = gappx;
	Client *c;

	for (c = m->clients; c; c = c->next)
		if (ISVISIBLE(c))
			n++;
	if (n > 0) /* override layout symbol */
		snprintf(m->ltsymbol, sizeof(m->ltsymbol), "[%d]", n);
	for (c = nexttiled(m->clients); c; c = nexttiled(c->next))
		resize(c, m->wx + gap, m->wy + gap, m->ww - 2 * c->bw - 2*gap, m->wh - 2 * c->bw - 2*gap, 0);
}

void
focusurgent(const Arg *arg)
{
	Client *c;
	int i;
	for(c=selmon->clients; c && !c->isurgent; c=c->next);
	if(c) {
		for(i=0; i < LENGTH(tags) && !((1 << i) & c->tags); i++);
		if(i < LENGTH(tags)) {
			const Arg a = {.ui = 1 << i};
			view(&a);
			focus(c);
		}
	}
}

void
movestack(const Arg *arg) {
	Client *c = NULL, *p = NULL, *pc = NULL, *i;

	if(arg->i > 0) {
		/* find the client after selmon->sel */
		for(c = selmon->sel->next; c && (!ISVISIBLE(c) || c->isfloating); c = c->next);
		if(!c)
			for(c = selmon->clients; c && (!ISVISIBLE(c) || c->isfloating); c = c->next);

	}
	else {
		/* find the client before selmon->sel */
		for(i = selmon->clients; i != selmon->sel; i = i->next)
			if(ISVISIBLE(i) && !i->isfloating)
				c = i;
		if(!c)
			for(; i; i = i->next)
				if(ISVISIBLE(i) && !i->isfloating)
					c = i;
	}
	/* find the client before selmon->sel and c */
	for(i = selmon->clients; i && (!p || !pc); i = i->next) {
		if(i->next == selmon->sel)
			p = i;
		if(i->next == c)
			pc = i;
	}

	/* swap c and selmon->sel selmon->clients in the selmon->clients list */
	if(c && c != selmon->sel) {
		Client *temp = selmon->sel->next==c?selmon->sel:selmon->sel->next;
		selmon->sel->next = c->next==selmon->sel?c:c->next;
		c->next = temp;

		if(p && p != c)
			p->next = c;
		if(pc && pc != selmon->sel)
			pc->next = selmon->sel;

		if(selmon->sel == selmon->clients)
			selmon->clients = c;
		else if(c == selmon->clients)
			selmon->clients = selmon->sel;

		arrange(selmon);
	}
}

void
resetlayout(const Arg *arg)
{
	Arg a;
	a.v = &layouts[0];
	setlayout(&a);
	a.f = mfact - selmon->mfact;
	setmfact(&a);
	a.i = nmaster - selmon->nmaster;
	incnmaster(&a);
	if (selmon->showbar != showbar)
		togglebar(NULL);
}

void
togglefullscreen(const Arg *arg)
{
	if(selmon->sel)
		setfullscreen(selmon->sel, !selmon->sel->isfullscreen);
}
