#ifndef CODE_FOCUS_HANDLER_H
#define CODE_FOCUS_HANDLER_H

#include "types.h"

class QPushButton;
class QtCodeArea;
class QtCodeFile;

class CodeFocusHandler
{
public:
	enum class Direction
	{
		UP,
		DOWN,
		LEFT,
		RIGHT
	};

	struct Focus
	{
		QtCodeFile* file = nullptr;
		QtCodeArea* area = nullptr;
		QPushButton* scopeLine = nullptr;
		size_t lineNumber = 0;
		Id locationId = 0;

		bool isEmpty() const
		{
			return file == nullptr && area == nullptr && scopeLine == nullptr && lineNumber == 0 && locationId == 0;
		}
	};

	void clear();

	void focus();
	void defocus();

	const Focus& getFocus() const;

	void setFocusedLocationId(QtCodeArea* area, size_t lineNumber, size_t columnNumber, Id locationId);
	void setFocusedScopeLine(QtCodeArea* area, QPushButton* scopeLine);
	void setFocusedFile(QtCodeFile* file);

	size_t getTargetColumn() const;

protected:
	Focus m_focus;
	Focus m_oldFocus;

	size_t m_targetColumn = 0;
};

#endif	  // CODE_FOCUS_HANDLER_H
