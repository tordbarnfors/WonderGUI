#include <string.h>


#include "slotlinks.h"

#include <wondergui.h>
#include <wondergfxstream.h>

#include <wg_string.h>

using namespace wg;

SlotLinksTest::SlotLinksTest()
{

	ADD_TEST(packPanelSlotLinksTest);
	ADD_TEST(flexPanelSlotLinksTest);

}

SlotLinksTest::~SlotLinksTest()
{
}


bool SlotLinksTest::init(std::ostream& output)
{
	return true;
}

//____ packPanelSlotLinksTest() _______________________________________________

bool SlotLinksTest::packPanelSlotLinksTest(std::ostream& output)
{
	auto pPanel = PackPanel::create();

	for( int i = 0 ; i < 10 ; i++ )
		pPanel->slots << Filler::create();

	auto link0 = pPanel->slots.makeLink(pPanel->slots.begin());
	auto pWidget0 = pPanel->slots[0].widget();

	auto link3 = pPanel->slots.makeLink(3);
	auto pWidget3 = pPanel->slots[3].widget();

	auto link8 = pPanel->slots.makeLink(8);
	auto pWidget8 = pPanel->slots[8].widget();


	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link8->widget() == pWidget8);

	pPanel->slots.insert(5, Filler::create() );

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link8->widget() == pWidget8);

	for( int i = 0 ; i < 300 ; i++ )
		pPanel->slots.insert(1, Filler::create() );

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link8->widget() == pWidget8);

	pPanel->slots.erase(2, 200);

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link8->widget() == pWidget8);

	TEST_ASSERT(link0.index() == 0);
	TEST_ASSERT(link0 == true);

	pPanel->slots.erase(0,1);
	TEST_ASSERT(link0.index() == -1);
	TEST_ASSERT(link0.ptr() == nullptr);
	TEST_ASSERT(link0 == false);

	link0 = link3;

	TEST_ASSERT(link0.index() == link3.index() );
	TEST_ASSERT(link0->widget() == pWidget3 );
	TEST_ASSERT(link0 == true);

	PackPanelSlot::Link linkX = pPanel->slots.makeLink(pPanel->slots.begin());

	pPanel = nullptr;

	TEST_ASSERT(link0 == false );
	TEST_ASSERT(link3 == false );
	TEST_ASSERT(link8 == false );



	return true;
}

//____ flexPanelSlotLinksTest() _______________________________________________

bool SlotLinksTest::flexPanelSlotLinksTest(std::ostream& output)
{
	auto pPanel = FlexPanel::create();

	for( int i = 0 ; i < 10 ; i++ )
		pPanel->slots << Filler::create();

	auto link0 = pPanel->slots.makeLink(0);
	auto pWidget0 = pPanel->slots[0].widget();

	auto link3 = pPanel->slots.makeLink(3);
	auto pWidget3 = pPanel->slots[3].widget();

	auto link5 = pPanel->slots.makeLink(5);
	auto pWidget5 = pPanel->slots[5].widget();

	auto link8 = pPanel->slots.makeLink(8);
	auto pWidget8 = pPanel->slots[8].widget();

	// Moving slots also shifts the slots in between.

	pPanel->slots.moveToFront(5);

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link5->widget() == pWidget5);
	TEST_ASSERT(link8->widget() == pWidget8);
	TEST_ASSERT(link5.index() == 0);
	TEST_ASSERT(link0.index() == 1);

	pPanel->slots.moveToBack(1);

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link5->widget() == pWidget5);
	TEST_ASSERT(link8->widget() == pWidget8);
	TEST_ASSERT(link0.index() == 9);

	pPanel->slots.moveBefore(link8.index(), link3.index());

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link5->widget() == pWidget5);
	TEST_ASSERT(link8->widget() == pWidget8);
	TEST_ASSERT(link8.index() + 1 == link3.index());

	pPanel->slots.moveBefore(link3.index(), link0.index());

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link5->widget() == pWidget5);
	TEST_ASSERT(link8->widget() == pWidget8);
	TEST_ASSERT(link3.index() + 1 == link0.index());

	// Pushing to front shifts all slots.

	pPanel->slots.pushFront(Filler::create());

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link5->widget() == pWidget5);
	TEST_ASSERT(link8->widget() == pWidget8);

	for( int i = 0 ; i < 300 ; i++ )
		pPanel->slots.pushFront(Filler::create());

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link5->widget() == pWidget5);
	TEST_ASSERT(link8->widget() == pWidget8);

	pPanel->slots.erase(0, 250);

	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link5->widget() == pWidget5);
	TEST_ASSERT(link8->widget() == pWidget8);

	// Releasing a widget kills its link but keeps the others.

	pWidget5->releaseFromParent();

	TEST_ASSERT(link5 == false);
	TEST_ASSERT(link0->widget() == pWidget0);
	TEST_ASSERT(link3->widget() == pWidget3);
	TEST_ASSERT(link8->widget() == pWidget8);

	// Links created after clear() must work.

	pPanel->slots.clear();

	TEST_ASSERT(link0 == false);
	TEST_ASSERT(link3 == false);
	TEST_ASSERT(link8 == false);

	for( int i = 0 ; i < 4 ; i++ )
		pPanel->slots << Filler::create();

	{
		auto * pLinkA = new FlexPanelSlot::Link(pPanel->slots.makeLink(1));
		auto * pLinkB = new FlexPanelSlot::Link(pPanel->slots.makeLink(2));
		auto pWidgetB = pPanel->slots[2].widget();

		pPanel->slots.clear();
		delete pLinkA;
		delete pLinkB;

		for( int i = 0 ; i < 4 ; i++ )
			pPanel->slots << Filler::create();

		auto linkC = pPanel->slots.makeLink(2);
		auto pWidgetC = pPanel->slots[2].widget();

		pPanel->slots.pushFront(Filler::create());

		TEST_ASSERT(linkC->widget() == pWidgetC);
		TEST_ASSERT(linkC.index() == 3);
	}

	return true;
}
