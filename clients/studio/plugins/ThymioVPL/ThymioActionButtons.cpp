#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QSlider>
#include <QTimeLine>
#include <QGraphicsProxyWidget>
#include <QGraphicsSvgItem>
#include <QtCore/qmath.h>

#include "ThymioButtons.h"

namespace Aseba
{
	// Move Action
	ThymioMoveAction::ThymioMoveAction( QGraphicsItem *parent ) :
		ThymioCard(false, false, parent)
	{
		setData(0, "action");
		setData(1, "move");

		for(int i=0; i<2; i++)
		{
			QSlider *s = new QSlider(Qt::Vertical);
			s->setRange(-10,10);
			s->setStyleSheet(
				"QSlider { border: 0px; padding: 0px; }"
				"QSlider::groove:vertical { "
					"border: 4px solid black; "
					"background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00FF00, stop:0.25 #FFFF00, stop:0.5 #FF0000, stop:0.75 #FFFF00, stop:1 #00FF00 ); }"
				"QSlider::handle:vertical { "
					"background: white; "
					"border: 4px solid black; height: 44px; margin: -4px; }"
			);
			s->setSliderPosition(0);

			QGraphicsProxyWidget *w = new QGraphicsProxyWidget(this);
			w->setAcceptDrops(false);
			w->setWidget(s);
			w->resize(48, 226);
			w->setPos(10+i*188, 15);
			
			sliders.push_back(s);
			
			connect(s, SIGNAL(valueChanged(int)), this, SLOT(valueChangeDetected()));
			connect(s, SIGNAL(valueChanged(int)), this, SLOT(updateIRButtonAndNotify()));
		}
		
		thymioBody = new ThymioBody(this, -70);
		thymioBody->setUp(false);
		thymioBody->setPos(128,128+14);
		thymioBody->setScale(0.2);
		
		timer = new QTimeLine(2000, this);
		timer->setFrameRange(0, 150);
		timer->setCurveShape(QTimeLine::LinearCurve);
		timer->setLoopCount(1);
		connect(timer, SIGNAL(frameChanged(int)), SLOT(frameChanged(int)));
	}

	ThymioMoveAction::~ThymioMoveAction()
	{
	}
	
	void ThymioMoveAction::frameChanged(int frame)
	{
		qreal pt[2];
		for(int i=0; i<2; i++) 
			pt[i] = (sliders[i]->value())*0.06*50; // [-30,30]
		
		const qreal step = frame/200.0;
		const qreal angle = (pt[0]-pt[1]-0.04)*3*step;
		const qreal center = -23.5*(pt[1]+pt[0])/(pt[1] == pt[0] ? 0.03 : (pt[1]-pt[0]));
		
		thymioBody->setPos(QPointF(center*(1-cos(-angle*3.14/180))+128,center*sin(-angle*3.14/180)+128+14));
		thymioBody->setRotation(angle);
	}

	void ThymioMoveAction::valueChangeDetected()
	{
		timer->stop();
		timer->start();
	}
	
	int ThymioMoveAction::getValue(int i) const
	{ 
		if (i<sliders.size()) 
			return sliders[i]->value()*50;
		return -1;
	}

	void ThymioMoveAction::setValue(int i, int value) 
	{ 
		if(i<sliders.size()) 
			sliders[i]->setSliderPosition(value/50);
	}
	
	// Color Action
	ThymioColorAction::ThymioColorAction( QGraphicsItem *parent, bool top) :
		ThymioCardWithBody(false, top, false, parent)
	{
		setData(0, "action");
		if (top)
			setData(1, "colortop");
		else
			setData(1, "colorbottom");

		const char *sliderColors[] = { "FF0000", "00FF00", "0000FF" };
		
		for(int i=0; i<3; i++)
		{
			QSlider *s = new QSlider(Qt::Horizontal);
			s->setRange(0,32);
			s->setStyleSheet(QString(
				"QSlider { border: 0px; padding: 0px; }"
				"QSlider::groove:horizontal { "
					"border: 4px solid black;"
					"background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #000000, stop:1 #%0); "
				"}"
				"QSlider::handle:horizontal { "
					"background: white; "
					"border: 4px solid black; width: 40px; margin: -4px;"
				"}"
				";").arg(sliderColors[i]));
			s->setSliderPosition(16);

			QGraphicsProxyWidget *w = new QGraphicsProxyWidget(this);
			w->setAcceptDrops(false);
			w->setWidget(s);
			w->resize(202, 48);
			w->setPos(27, 60+i*64);
			
			sliders.push_back(s);
			
			connect(s, SIGNAL(valueChanged(int)), this, SLOT(valueChangeDetected()));
			connect(s, SIGNAL(valueChanged(int)), this, SLOT(updateIRButtonAndNotify()));
		}
	}

	int ThymioColorAction::getValue(int i) const
	{ 
		if(i<sliders.size()) 
			return sliders[i]->value(); 
		return -1;
	}
	
	void ThymioColorAction::setValue(int i, int status) 
	{ 
		if(i<sliders.size()) 
			sliders[i]->setSliderPosition(status);
	}
	
	void ThymioColorAction::valueChangeDetected()
	{
		update();
	}
	
	void ThymioColorAction::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
	{
		ThymioCard::paint(painter, option, widget);
		const QColor bodyColor( sliders[0]->value()*5.46875+80, 
								sliders[1]->value()*5.46875+80, 
								sliders[2]->value()*5.46875+80);
		ThymioBody::drawBody(painter, 128, 128, up, bodyColor);
	}
	
	ThymioColorTopAction::ThymioColorTopAction(QGraphicsItem *parent):
		ThymioColorAction(parent, true)
	{}
	
	ThymioColorBottomAction::ThymioColorBottomAction(QGraphicsItem *parent):
		ThymioColorAction(parent, false)
	{}
	
	// Sound Action
	ThymioSoundAction::ThymioSoundAction(QGraphicsItem *parent) :
		ThymioCardWithBody(false, true, false, parent)
	{
		setData(0, "action");
		setData(1, "sound");
		
		notes[0] = 0;
		notes[1] = 2;
		notes[2] = 3;
		notes[3] = 5;
		notes[4] = 5;
		notes[5] = 3;
		notes[6] = 2;
		notes[7] = 0;
		
		new QGraphicsSvgItem (":/images/notes.svgz", this);
	}
	
	void ThymioSoundAction::paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
	{
		Q_UNUSED(option);
		Q_UNUSED(widget);
		
		ThymioCardWithBody::paint(painter, option, widget);
		
		painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
		for (int row = 0; row < 6; ++row)
		{
			for (int col = 0; col < 8; ++col)
			{
				if (notes[col] == 5-row)
					painter->setBrush(Qt::black);
				else
					painter->setBrush(QColor::fromHsv(row*42, 80, 255));
				painter->drawRect(28 + col*25, 88 + row*25, 25, 25);
			}
		}
	}
	
	void ThymioSoundAction::mousePressEvent(QGraphicsSceneMouseEvent * event)
	{
		if (event->button() == Qt::LeftButton)
		{
			const QPointF& pos(event->pos());
			if (QRectF(28,78,200,150).contains(pos))
			{
				notes[(int(pos.x())-28)/25] = 5-(int(pos.y())-88)/25;
			}
			update();
			updateIRButtonAndNotify();
		}
	}
	
	int ThymioSoundAction::getValue(int i) const
	{ 
		if (i<8) 
			return notes[i]; 
		return -1;
	}
	
	// TODO: FIXME: use size_t for index
	void ThymioSoundAction::setValue(int i, int value) 
	{ 
		if (i<8) 
		{
			notes[i] = value;
			update();
			updateIRButtonAndNotify();
		}
	}

	// Memory Action
	ThymioMemoryAction::ThymioMemoryAction(QGraphicsItem *parent) : 
		ThymioCardWithBody(false, true, false, parent)
	{
		setData(0, "action");
		setData(1, "memory");
		
		for(uint i=0; i<4; i++)
		{
			//ThymioClickableButton *button = new ThymioClickableButton(QRectF(-15,-30,30,60), THYMIO_CIRCULAR_BUTTON, 2, this);//THYMIO_RECTANGULAR_BUTTON , 2, this);
			//button->setPos(128 + (2-i)*(i%2)*60, 138 + (i-1)*((i+1)%2)*60); 
			//button->setRotation(90*(i+1));
			ThymioClickableButton *button = new ThymioClickableButton(QRectF(-20,-20,40,40), THYMIO_CIRCULAR_BUTTON, this, Qt::lightGray, Qt::darkGray);
			button->setPos(98 + (i%2)*60, 98 + (i/2)*60);
			button->addState(QColor(255,128,0));
			button->addState(Qt::white);

			thymioButtons.push_back(button);
			connect(button, SIGNAL(stateChanged()), this, SLOT(updateIRButtonAndNotify()));
		}
	}	

};

