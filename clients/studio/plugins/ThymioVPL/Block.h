#ifndef VPL_CARD_H
#define VPL_CARD_H

#include <QObject>
#include <QGraphicsObject>
#include <QList>
#include <QDomElement>

class QMimeData;
class QSlider;
class QDomDocument;

namespace Aseba { namespace ThymioVPL
{
	/** \addtogroup studio */
	/*@{*/
	
	class GeometryShapeButton;
	
	/**
		An "event" or "action" block.
		
		These blocks have a type (event or action) and a name (prox, etc.)
		and may provide several values (set/get by setValue()/getValue()).
		These values are set by the user through buttons (typically
		GeometryShapeButton), sliders, or specific widgets.
	*/
	class Block : public QGraphicsObject
	{
		Q_OBJECT
		
	public:
		// TODO: move that somewhere else
		class ThymioBody : public QGraphicsItem
		{
		public:
			ThymioBody(QGraphicsItem *parent = 0, int yShift = 0) : QGraphicsItem(parent), bodyColor(Qt::white), yShift(yShift), up(true) { }
			
			void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
			QRectF boundingRect() const { return QRectF(-128, -128+yShift, 256, 256); }
			void setUp(bool u) { up = u; }
			
			static void drawBody(QPainter * painter, int xShift, int yShift, bool up, const QColor& bodyColor);
			
			QColor bodyColor;
			
		private:
			const int yShift;
			bool up;
		};
		
		static Block* createBlock(const QString& name, bool advanced=false, QGraphicsItem *parent=0);
		
		Block(bool isEvent, const QString& name, QGraphicsItem *parent=0);
		virtual ~Block();
		
		virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
		QRectF boundingRect() const { return QRectF(0, 0, 256, 256); }
		virtual QPixmap image(qreal factor=1);

		// TODO: remove?
		void setParentID(int id) { parentID = id; }
		int getParentID() const { return parentID; }
		
		QString getType() const { return isEvent ? "event" : "action"; }
		QString getName() const { return name; }
		
		virtual unsigned valuesCount() const = 0;
		virtual int getValue(unsigned i) const = 0;
		virtual void setValue(unsigned i, int value) = 0;
		virtual bool isAnyValueSet() const;
		
		virtual bool isAnyAdvancedFeature() const { return false; }
		virtual void setAdvanced(bool advanced) {}
		
		QMimeData* mimeData() const;
		QDomElement serialize(QDomDocument& document) const;
		static Block* deserialize(const QDomElement& element, bool advanced);
		static Block* deserialize(const QByteArray& data, bool advanced);
		static QString deserializeType(const QByteArray& data);
		static QString deserializeName(const QByteArray& data);

	signals:
		void contentChanged();
		
	public:
		const bool isEvent;
		const QString name;
		bool beingDragged;

	protected:
		void render(QPainter& painter);
		virtual void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
		
		// TODO: remove?
		int parentID;
	};
	
	class BlockWithNoValues: public Block
	{
	public:
		BlockWithNoValues(bool isEvent, const QString& name, QGraphicsItem *parent);
		
		virtual unsigned valuesCount() const { return 0; }
		virtual int getValue(unsigned i) const { return -1; }
		virtual void setValue(unsigned i, int value) {}
	};
	
	class BlockWithBody: public Block
	{
	public:
		BlockWithBody(bool isEvent, const QString& name, bool up, QGraphicsItem *parent);
		
		virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
	
	public:
		const bool up;
		
	protected:
		QColor bodyColor;
	};
	
	class BlockWithButtons: public BlockWithBody
	{
	public:
		BlockWithButtons(bool isEvent, const QString& name, bool up, QGraphicsItem *parent);
		
		virtual unsigned valuesCount() const;
		virtual int getValue(unsigned i) const;
		virtual void setValue(unsigned i, int value);
		
	protected:
		QList<GeometryShapeButton*> buttons;
	};
	
	class BlockWithButtonsAndRange: public BlockWithButtons
	{
	public:
		BlockWithButtonsAndRange(bool isEvent, const QString& name, bool up, int lowerBound, int upperBound, int defaultLow, int defaultHigh, bool advanced, QGraphicsItem *parent);
		
		virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);
		
		virtual unsigned valuesCount() const;
		virtual int getValue(unsigned i) const;
		virtual void setValue(unsigned i, int value);
		virtual bool isAnyValueSet() const;
		
		virtual bool isAnyAdvancedFeature() const;
		virtual void setAdvanced(bool advanced);
		
	public:
		const int lowerBound;
		const int upperBound;
		const int range;
		const int defaultLow;
		const int defaultHigh;
	
	protected:
		virtual void mousePressEvent( QGraphicsSceneMouseEvent * event);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
		
		QRectF rangeRect() const;
		float pixelToVal(float pixel) const;
		float valToPixel(float val) const;
		
	protected:
		int low; // low activation threshold (at right)
		int high; // high activation threshold (at left)
		bool lastPressedIn; // whether last mouse press event was in
		bool showRangeControl; // whether we are in advanced mode
	};
	
	class StateFilterBlock: public BlockWithButtons
	{
	public:
		StateFilterBlock(bool isEvent, const QString& name, QGraphicsItem *parent=0);
		
		virtual bool isAnyAdvancedFeature() const { return true; }
	};
	
	/*@}*/
} } // namespace ThymioVPL / namespace Aseba

#endif
