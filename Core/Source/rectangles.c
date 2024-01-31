#include "core/math/rectangles.h"
#include "core/cunit.h"

private bool Contains(irectangle, irectangle);
private bool CanFit(irectangle, irectangle);
private void RunUnitTests();
private tuple(irectangle, irectangle) BisectRectAlongXAxis(irectangle rect, int xBisection);
private tuple(irectangle, irectangle) BisectRectAlongYAxis(irectangle rect, int xBisection);
private tuple(irectangle, irectangle) SubtractRectangle(irectangle source, irectangle value);

const struct _rectangleMethods Rectangles = {
	.CanFit = CanFit,
	.Contains = Contains,
	.RunUnitTests = RunUnitTests,
	.BisectRectAlongYAxis = BisectRectAlongYAxis,
	.BisectRectAlongXAxis = BisectRectAlongXAxis,
	.SubtractRectangle = SubtractRectangle
};

#define area(rect) ((rect).Width * (rect).Height)

private bool Contains(irectangle left, irectangle right)
{
	const bool positionInside = right.x >= left.x && right.y >= left.y;

	const ivector2 rightsTopRightCorner = { right.x + right.Width, right.y + right.Height };
	const ivector2 leftsTopRightCorner = { left.x + left.Width, left.y + left.Height };

	const bool sizeInside = positionInside && rightsTopRightCorner.x <= leftsTopRightCorner.x && rightsTopRightCorner.y <= leftsTopRightCorner.y;

	return sizeInside;
}

private bool CanFit(irectangle destination, irectangle rect)
{
	return destination.Width >= rect.Width && destination.Height >= rect.Height;
}

//                     xBisection
// +-----------------------+------------------------+
// |         first         |         second         |
// +-----------------------+------------------------+
private tuple(irectangle, irectangle) BisectRectAlongXAxis(irectangle rect, int xBisection)
{
	int leftWidth = (xBisection - rect.x);

	irectangle right = {
		.y = rect.y,
		.x = xBisection,
		.Width = rect.Width - leftWidth,
		.Height = rect.Height
	};

	rect.Width = leftWidth;

	return (tuple(irectangle, irectangle)) { rect, right };
}

//                       top
// +------------------------------------------------+
// |                     first                      |
// +------------------------------------------------+ yBisection
// |                     second                     |
// +------------------------------------------------+
private tuple(irectangle, irectangle) BisectRectAlongYAxis(irectangle top, int yBisection)
{
	int topHeight = yBisection - top.y;

	irectangle bottom = {
		.y = yBisection,
		.x = top.x,
		.Width = top.Width,
		.Height = top.Width - topHeight
	};

	top.Height = topHeight;

	return (tuple(irectangle, irectangle)) { top, bottom };
}

// Subtracts value from the source rectangle, returns a tuple of the remainder. The first rectangle returned
// is right rectangle and the second rectangle returned is the bottom rectangle.
//                       source
// +-----------------------+------------------------+
// |         value         |       first            |
// +-----------------------+------------------------+
// |                     second                     |
// +------------------------------------------------+
private tuple(irectangle, irectangle) SubtractRectangle(irectangle source, irectangle value)
{
	// bisect along y first so we don't end up with two rectangles on the bottom cuz that would be weird
	tuple(irectangle, irectangle) yBisection = BisectRectAlongYAxis(source, value.y + value.Height);

	irectangle bottom = yBisection.Second;

	tuple(irectangle, irectangle) xBisection = BisectRectAlongXAxis(yBisection.First, value.x + value.Width);

	irectangle right = xBisection.Second;

	return (tuple(irectangle, irectangle)) { right, bottom };
}

TEST(xBisectWorks)
{
	irectangle source = {
		.x = 0,
		.y = 0,
		.Width = 1024,
		.Height = 1024
	};

	tuple(irectangle, irectangle) result = BisectRectAlongXAxis(source, 512);

	const irectangle left = result.First;
	const irectangle right = result.Second;

	IsEqual(left.x, 0, "%d");
	IsEqual(left.y, 0, "%d");
	IsEqual(left.Width, 512, "%d");
	IsEqual(left.Height, 1024, "%d");

	IsEqual(right.x, 512, "%d");
	IsEqual(right.y, 0, "%d");
	IsEqual(right.Width, 512, "%d");
	IsEqual(right.Height, 1024, "%d");

	return true;
}

TEST(yBisectWorks)
{
	irectangle source = {
		.x = 0,
		.y = 0,
		.Width = 1024,
		.Height = 1024
	};

	tuple(irectangle, irectangle) result = BisectRectAlongYAxis(source, 512);

	const irectangle top = result.First;
	const irectangle bottom = result.Second;

	IsEqual(top.x, 0, "%d");
	IsEqual(top.y, 0, "%d");
	IsEqual(top.Width, 1024, "%d");
	IsEqual(top.Height, 512, "%d");

	IsEqual(bottom.x, 0, "%d");
	IsEqual(bottom.y, 512, "%d");
	IsEqual(bottom.Width, 1024, "%d");
	IsEqual(bottom.Height, 512, "%d");

	return true;
}

TEST(SubtractWorks)
{
	irectangle source = {
		.x = 0,
		.y = 0,
		.Width = 1024,
		.Height = 1024
	};

	irectangle smallerRect =
	{
		.x = 0,
		.y = 0,
		.Width = 64,
		.Height = 64
	};

	tuple(irectangle, irectangle) result = SubtractRectangle(source, smallerRect);

	const irectangle right = result.First;
	const irectangle bottom = result.Second;

	IsEqual(right.x, 64, "%d");
	IsEqual(right.y, 0, "%d");
	IsEqual(right.Width, 1024 - 64, "%d");
	IsEqual(right.Height, 64, "%d");

	IsEqual(bottom.x, 0, "%d");
	IsEqual(bottom.y, 64, "%d");
	IsEqual(bottom.Width, 1024, "%d");
	IsEqual(bottom.Height, 1024 - 64, "%d");

	return true;
}

TEST_SUITE(
	RunUnitTests,
	APPEND_TEST(yBisectWorks)
	APPEND_TEST(xBisectWorks)
	APPEND_TEST(SubtractWorks)
);