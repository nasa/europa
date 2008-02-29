/*
   Test program for TinyXML.
*/


#include "tinyxml.h"
using namespace EUROPA;

#ifdef TIXML_USE_STL
	#include <iostream>
	#include <sstream>
	using namespace std;
#else
	#include <stdio.h>
#endif

#if defined( WIN32 ) && defined( TUNE )
	#include <windows.h>
	// Apologies to non-windows users! But I need some good timers for
	// profiling, and these are very platform specific.
	__int64 start;
	__int64 end;
	__int64 freq;
#endif

static int gPass = 0;
static int gFail = 0;


bool XmlTest (const char* testString, const char* expected, const char* found, bool noEcho = false)
{
	bool pass = !strcmp( expected, found );
	if ( pass )
		printf ("[pass]");
	else
		printf ("[fail]");

	if ( noEcho )
		printf (" %s\n", testString);
	else
		printf (" %s [%s][%s]\n", testString, expected, found);

	if ( pass )
		++gPass;
	else
		++gFail;
	return pass;
}


bool XmlTest( const char* testString, int expected, int found, bool noEcho = false )
{
	bool pass = ( expected == found );
	if ( pass )
		printf ("[pass]");
	else
		printf ("[fail]");

	if ( noEcho )
		printf (" %s\n", testString);
	else
		printf (" %s [%d][%d]\n", testString, expected, found);

	if ( pass )
		++gPass;
	else
		++gFail;
	return pass;
}


//
// This file demonstrates some basic functionality of TinyXml.
// Note that the example is very contrived. It presumes you know
// what is in the XML file. But it does test the basic operations,
// and show how to add and remove nodes.
//

int main()
{
	//
	// We start with the 'demoStart' todo list. Process it. And
	// should hopefully end up with the todo list as illustrated.
	//
	const char* demoStart =
		"<?xml version=\"1.0\"  standalone='no' >\n"
		"<!-- Our to do list data -->"
		"<ToDo>\n"
		"<!-- Do I need a secure PDA? -->\n"
		"<Item priority=\"1\" distance='close'> Go to the <bold>Toy store!</bold></Item>"
		"<Item priority=\"2\" distance='none'> Do bills   </Item>"
		"<Item priority=\"2\" distance='far &amp; back'> Look for Evil Dinosaurs! </Item>"
		"</ToDo>";

#ifdef TIXML_USE_STL
	/*	What the todo list should look like after processing.
		In stream (no formatting) representation. */
	const char* demoEnd =
		"<?xml version=\"1.0\" standalone=\"no\" ?>"
		"<!-- Our to do list data -->"
		"<ToDo>"
		"<!-- Do I need a secure PDA? -->"
		"<Item priority=\"2\" distance=\"close\">Go to the"
		"<bold>Toy store!"
		"</bold>"
		"</Item>"
		"<Item priority=\"1\" distance=\"far\">Talk to:"
		"<Meeting where=\"School\">"
		"<Attendee name=\"Marple\" position=\"teacher\" />"
		"<Attendee name=\"Vo&#x82;\" position=\"counselor\" />"
		"</Meeting>"
		"<Meeting where=\"Lunch\" />"
		"</Item>"
		"<Item priority=\"2\" distance=\"here\">Do bills"
		"</Item>"
		"</ToDo>";
#endif

	// The example parses from the character string (above):
	#if defined( WIN32 ) && defined( TUNE )
	QueryPerformanceCounter( (LARGE_INTEGER*) (&start) );
	#endif

	{
		// Write to a file and read it back, to check file I/O.

		TiXmlDocument doc( "demotest.xml" );
		doc.Parse( demoStart );

		if ( doc.Error() )
		{
			printf( "Error in %s: %s\n", doc.Value(), doc.ErrorDesc() );
			exit( 1 );
		}
		doc.SaveFile();
	}

	TiXmlDocument doc( "demotest.xml" );
	bool loadOkay = doc.LoadFile();

	if ( !loadOkay )
	{
		printf( "Could not load test file 'demotest.xml'. Error='%s'. Exiting.\n", doc.ErrorDesc() );
		exit( 1 );
	}

	printf( "** Demo doc read from disk: ** \n\n" );
	doc.Print( stdout );

	TiXmlNode* node = 0;
	TiXmlElement* todoElement = 0;
	TiXmlElement* itemElement = 0;


	// --------------------------------------------------------
	// An example of changing existing attributes, and removing
	// an element from the document.
	// --------------------------------------------------------

	// Get the "ToDo" element.
	// It is a child of the document, and can be selected by name.
	node = doc.FirstChild( "ToDo" );
	assert( node );
	todoElement = node->ToElement();
	assert( todoElement  );

	// Going to the toy store is now our second priority...
	// So set the "priority" attribute of the first item in the list.
	node = todoElement->FirstChildElement();	// This skips the "PDA" comment.
	assert( node );
	itemElement = node->ToElement();
	assert( itemElement  );
	itemElement->SetAttribute( "priority", 2 );

	// Change the distance to "doing bills" from
	// "none" to "here". It's the next sibling element.
	itemElement = itemElement->NextSiblingElement();
	assert( itemElement );
	itemElement->SetAttribute( "distance", "here" );

	// Remove the "Look for Evil Dinosours!" item.
	// It is 1 more sibling away. We ask the parent to remove
	// a particular child.
	itemElement = itemElement->NextSiblingElement();
	todoElement->RemoveChild( itemElement );

	itemElement = 0;

	// --------------------------------------------------------
	// What follows is an example of created elements and text
	// nodes and adding them to the document.
	// --------------------------------------------------------

	// Add some meetings.
	TiXmlElement item( "Item" );
	item.SetAttribute( "priority", "1" );
	item.SetAttribute( "distance", "far" );

	TiXmlText text( "Talk to:" );

	TiXmlElement meeting1( "Meeting" );
	meeting1.SetAttribute( "where", "School" );

	TiXmlElement meeting2( "Meeting" );
	meeting2.SetAttribute( "where", "Lunch" );

	TiXmlElement attendee1( "Attendee" );
	attendee1.SetAttribute( "name", "Marple" );
	attendee1.SetAttribute( "position", "teacher" );

	TiXmlElement attendee2( "Attendee" );
	attendee2.SetAttribute( "name", "Vo&#x82;" );
	attendee2.SetAttribute( "position", "counselor" );

	// Assemble the nodes we've created:
	meeting1.InsertEndChild( attendee1 );
	meeting1.InsertEndChild( attendee2 );

	item.InsertEndChild( text );
	item.InsertEndChild( meeting1 );
	item.InsertEndChild( meeting2 );

	// And add the node to the existing list after the first child.
	node = todoElement->FirstChild( "Item" );
	assert( node );
	itemElement = node->ToElement();
	assert( itemElement );

	todoElement->InsertAfterChild( itemElement, item );

	printf( "\n** Demo doc processed: ** \n\n" );
	doc.Print( stdout );


#ifdef TIXML_USE_STL
	printf( "** Demo doc processed to stream: ** \n\n" );
	cout << doc << endl << endl;
#endif

	// --------------------------------------------------------
	// Different tests...do we have what we expect?
	// --------------------------------------------------------

	int count = 0;
	TiXmlElement*	element;

	//////////////////////////////////////////////////////

#ifdef TIXML_USE_STL
	cout << "** Basic structure. **\n";
	ostringstream outputStream( ostringstream::out );
	outputStream << doc;
	XmlTest( "Output stream correct.",	string( demoEnd ).c_str(),
										outputStream.str().c_str(), true );
#endif

	node = doc.RootElement();
	XmlTest( "Root element exists.", true, ( node != 0 && node->ToElement() ) );
	XmlTest ( "Root element value is 'ToDo'.", "ToDo",  node->Value());

	node = node->FirstChild();
	XmlTest( "First child exists & is a comment.", true, ( node != 0 && node->ToComment() ) );
	node = node->NextSibling();
	XmlTest( "Sibling element exists & is an element.", true, ( node != 0 && node->ToElement() ) );
	XmlTest ( "Value is 'Item'.", "Item", node->Value() );

	node = node->FirstChild();
	XmlTest ( "First child exists.", true, ( node != 0 && node->ToText() ) );
	XmlTest ( "Value is 'Go to the'.", "Go to the", node->Value() );


	//////////////////////////////////////////////////////
	printf ("\n** Iterators. **\n");

	// Walk all the top level nodes of the document.
	count = 0;
	for( node = doc.FirstChild();
		 node;
		 node = node->NextSibling() )
	{
		count++;
	}
	XmlTest( "Top level nodes, using First / Next.", 3, count );

	count = 0;
	for( node = doc.LastChild();
		 node;
		 node = node->PreviousSibling() )
	{
		count++;
	}
	XmlTest( "Top level nodes, using Last / Previous.", 3, count );

	// Walk all the top level nodes of the document,
	// using a different sytax.
	count = 0;
	for( node = doc.IterateChildren( 0 );
		 node;
		 node = doc.IterateChildren( node ) )
	{
		count++;
	}
	XmlTest( "Top level nodes, using IterateChildren.", 3, count );

	// Walk all the elements in a node.
	count = 0;
	for( element = todoElement->FirstChildElement();
		 element;
		 element = element->NextSiblingElement() )
	{
		count++;
	}
	XmlTest( "Children of the 'ToDo' element, using First / Next.",
		3, count );

	// Walk all the elements in a node by value.
	count = 0;
	for( node = todoElement->FirstChild( "Item" );
		 node;
		 node = node->NextSibling( "Item" ) )
	{
		count++;
	}
	XmlTest( "'Item' children of the 'ToDo' element, using First/Next.", 3, count );

	count = 0;
	for( node = todoElement->LastChild( "Item" );
		 node;
		 node = node->PreviousSibling( "Item" ) )
	{
		count++;
	}
	XmlTest( "'Item' children of the 'ToDo' element, using Last/Previous.", 3, count );

#ifdef TIXML_USE_STL
	{
		cout << "\n** Parsing. **\n";
		istringstream parse0( "<Element0 attribute0='foo0' attribute1= noquotes attribute2 = '&gt;' />" );
		TiXmlElement element0( "default" );
		parse0 >> element0;

		XmlTest ( "Element parsed, value is 'Element0'.", "Element0", element0.Value() );
		XmlTest ( "Reads attribute 'attribute0=\"foo0\"'.", "foo0", element0.Attribute( "attribute0" ));
		XmlTest ( "Reads incorrectly formatted 'attribute1=noquotes'.", "noquotes", element0.Attribute( "attribute1" ) );
		XmlTest ( "Read attribute with entity value '>'.", ">", element0.Attribute( "attribute2" ) );
	}
#endif

	{
		const char* error =	"<?xml version=\"1.0\" standalone=\"no\" ?>\n"
							"<passages count=\"006\" formatversion=\"20020620\">\n"
							"    <wrong error>\n"
							"</passages>";

        TiXmlDocument doc;
		doc.Parse( error );
		XmlTest( "Error row", doc.ErrorRow(), 3 );
		XmlTest( "Error column", doc.ErrorCol(), 17 );
		//printf( "error=%d id='%s' row %d col%d\n", (int) doc.Error(), doc.ErrorDesc(), doc.ErrorRow()+1, doc.ErrorCol() + 1 );

	}
	{
		const char* str =	"\t<?xml version=\"1.0\" standalone=\"no\" ?>\t<room doors='2'>\n"
							"  <!-- Silly example -->\n"
							"    <door wall='north'>A great door!</door>\n"
							"\t<door wall='east'/>"
							"</room>";

        TiXmlDocument doc;
		doc.Parse( str );

		TiXmlHandle docHandle( &doc );
		TiXmlHandle roomHandle = docHandle.FirstChildElement( "room" );
		TiXmlHandle commentHandle = docHandle.FirstChildElement( "room" ).FirstChild();
		TiXmlHandle textHandle = docHandle.FirstChildElement( "room" ).ChildElement( "door", 0 ).FirstChild();
		TiXmlHandle door0Handle = docHandle.FirstChildElement( "room" ).ChildElement( 0 );
		TiXmlHandle door1Handle = docHandle.FirstChildElement( "room" ).ChildElement( 1 );

		assert( docHandle.Node() );
		assert( roomHandle.Element() );
		assert( commentHandle.Node() );
		assert( textHandle.Text() );
		assert( door0Handle.Element() );
		assert( door1Handle.Element() );

		TiXmlDeclaration* declaration = doc.FirstChild()->ToDeclaration();
		assert( declaration );
		TiXmlElement* room = roomHandle.Element();
		assert( room );
		TiXmlAttribute* doors = room->FirstAttribute();
		assert( doors );
		TiXmlText* text = textHandle.Text();
		TiXmlComment* comment = commentHandle.Node()->ToComment();
		assert( comment );
		TiXmlElement* door0 = door0Handle.Element();
		TiXmlElement* door1 = door1Handle.Element();

		XmlTest( "Location tracking: Declaration row", declaration->Row(), 1 );
		XmlTest( "Location tracking: Declaration col", declaration->Column(), 5 );
		XmlTest( "Location tracking: room row", room->Row(), 1 );
		XmlTest( "Location tracking: room col", room->Column(), 45 );
		XmlTest( "Location tracking: doors row", doors->Row(), 1 );
		XmlTest( "Location tracking: doors col", doors->Column(), 51 );
		XmlTest( "Location tracking: Comment row", comment->Row(), 2 );
		XmlTest( "Location tracking: Comment col", comment->Column(), 3 );
		XmlTest( "Location tracking: text row", text->Row(), 3 ); 
		XmlTest( "Location tracking: text col", text->Column(), 24 );
		XmlTest( "Location tracking: door0 row", door0->Row(), 3 );
		XmlTest( "Location tracking: door0 col", door0->Column(), 5 );
		XmlTest( "Location tracking: door1 row", door1->Row(), 4 );
		XmlTest( "Location tracking: door1 col", door1->Column(), 5 );
	}
	{
		const char* str =	"\t<?xml version=\"1.0\" standalone=\"no\" ?>\t<room doors='2'>\n"
							"</room>";

        TiXmlDocument doc;
		doc.SetTabSize( 8 );
		doc.Parse( str );

		TiXmlHandle docHandle( &doc );
		TiXmlHandle roomHandle = docHandle.FirstChildElement( "room" );

		assert( docHandle.Node() );
		assert( roomHandle.Element() );

		TiXmlElement* room = roomHandle.Element();
		assert( room );
		TiXmlAttribute* doors = room->FirstAttribute();
		assert( doors );

		XmlTest( "Location tracking: Tab 8: room row", room->Row(), 1 );
		XmlTest( "Location tracking: Tab 8: room col", room->Column(), 49 );
		XmlTest( "Location tracking: Tab 8: doors row", doors->Row(), 1 );
		XmlTest( "Location tracking: Tab 8: doors col", doors->Column(), 55 );
	}

	{
		const char* str = "<doc attr0='1' attr1='2.0' attr2='foo' />";

		TiXmlDocument doc;
		doc.Parse( str );

		TiXmlElement* ele = doc.FirstChildElement();

		int iVal, result;
		double dVal;

		result = ele->QueryDoubleAttribute( "attr0", &dVal );
		XmlTest( "Query attribute: int as double", result, TIXML_SUCCESS );
		XmlTest( "Query attribute: int as double", (int)dVal, 1 );
		result = ele->QueryDoubleAttribute( "attr1", &dVal );
		XmlTest( "Query attribute: double as double", (int)dVal, 2 );
		result = ele->QueryIntAttribute( "attr1", &iVal );
		XmlTest( "Query attribute: double as int", result, TIXML_SUCCESS );
		XmlTest( "Query attribute: double as int", iVal, 2 );
		result = ele->QueryIntAttribute( "attr2", &iVal );
		XmlTest( "Query attribute: not a number", result, TIXML_WRONG_TYPE );
		result = ele->QueryIntAttribute( "bar", &iVal );
		XmlTest( "Query attribute: does not exist", result, TIXML_NO_ATTRIBUTE );
	}

#ifdef TIXML_USE_STL
	{
		//////////////////////////////////////////////////////
		cout << "\n** Streaming. **\n";

		// Round trip check: stream in, then stream back out to verify. The stream
		// out has already been checked, above. We use the output

		istringstream inputStringStream( outputStream.str() );
		TiXmlDocument document0;

		inputStringStream >> document0;

		ostringstream outputStream0( ostringstream::out );
		outputStream0 << document0;

		XmlTest( "Stream round trip correct.",	string( demoEnd ).c_str(), 
												outputStream0.str().c_str(), true );

		std::string str;
		str << document0;

		XmlTest( "String printing correct.", string( demoEnd ).c_str(), 
											 str.c_str(), true );
	}
#endif

	//////////////////////////////////////////////////////
#ifdef TIXML_USE_STL
	printf ("\n** Parsing, no Condense Whitespace **\n");
	TiXmlBase::SetCondenseWhiteSpace( false );

	istringstream parse1( "<start>This  is    \ntext</start>" );
	TiXmlElement text1( "text" );
	parse1 >> text1;

	XmlTest ( "Condense white space OFF.", "This  is    \ntext",
				text1.FirstChild()->Value(),
				true );
#endif

	//////////////////////////////////////////////////////
	printf ("\n** Bug regression tests **\n");

	// InsertBeforeChild and InsertAfterChild causes crash.
	{
		TiXmlElement parent( "Parent" );
		TiXmlElement childText0( "childText0" );
		TiXmlElement childText1( "childText1" );
		TiXmlNode* childNode0 = parent.InsertEndChild( childText0 );
		TiXmlNode* childNode1 = parent.InsertBeforeChild( childNode0, childText1 );

		XmlTest( "Test InsertBeforeChild on empty node.", ( childNode1 == parent.FirstChild() ), true );
	}

	{
		// InsertBeforeChild and InsertAfterChild causes crash.
		TiXmlElement parent( "Parent" );
		TiXmlElement childText0( "childText0" );
		TiXmlElement childText1( "childText1" );
		TiXmlNode* childNode0 = parent.InsertEndChild( childText0 );
		TiXmlNode* childNode1 = parent.InsertAfterChild( childNode0, childText1 );

		XmlTest( "Test InsertAfterChild on empty node. ", ( childNode1 == parent.LastChild() ), true );
	}

	// Reports of missing constructors, irregular string problems.
	{
		// Missing constructor implementation. No test -- just compiles.
		TiXmlText text( "Missing" );

		#ifdef TIXML_USE_STL
			// Missing implementation:
			TiXmlDocument doc;
			string name = "missing";
			doc.LoadFile( name );

			TiXmlText textSTL( name );
		#else
			// verifing some basic string functions:
			TiXmlString a;
			TiXmlString b = "Hello";
			TiXmlString c = "ooga";

			c = " World!";
			a = b;
			a += c;
			a = a;

			XmlTest( "Basic TiXmlString test. ", "Hello World!", a.c_str() );
		#endif
 	}

	// Long filenames crashing STL version
	{
		TiXmlDocument doc( "midsummerNightsDreamWithAVeryLongFilenameToConfuseTheStringHandlingRoutines.xml" );
		bool loadOkay = doc.LoadFile();
		loadOkay = true;	// get rid of compiler warning.
		// Won't pass on non-dev systems. Just a "no crash" check.
		//XmlTest( "Long filename. ", true, loadOkay );
	}

	{
		// Entities not being written correctly.
		// From Lynn Allen

		const char* passages =
			"<?xml version=\"1.0\" standalone=\"no\" ?>"
			"<passages count=\"006\" formatversion=\"20020620\">"
				"<psg context=\"Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;."
				" It also has &lt;, &gt;, and &amp;, as well as a fake &#xA9;.\"> </psg>"
			"</passages>";

		TiXmlDocument doc( "passages.xml" );
		doc.Parse( passages );
		TiXmlElement* psg = doc.RootElement()->FirstChildElement();
		const char* context = psg->Attribute( "context" );

		XmlTest( "Entity transformation: read. ",
					"Line 5 has \"quotation marks\" and 'apostrophe marks'."
					" It also has <, >, and &, as well as a fake \xA9.",
					context,
					true );

		FILE* textfile = fopen( "textfile.txt", "w" );
		if ( textfile )
		{
			psg->Print( textfile, 0 );
			fclose( textfile );
		}
		textfile = fopen( "textfile.txt", "r" );
		assert( textfile );
		if ( textfile )
		{
			char buf[ 1024 ];
			fgets( buf, 1024, textfile );
			XmlTest( "Entity transformation: write. ",
					 "<psg context=\'Line 5 has &quot;quotation marks&quot; and &apos;apostrophe marks&apos;."
					 " It also has &lt;, &gt;, and &amp;, as well as a fake &#xA9;.' />",
					 buf,
					 true );
		}
		fclose( textfile );
	}

    {
		FILE* textfile = fopen( "test5.xml", "w" );
		if ( textfile )
		{
            fputs("<?xml version='1.0'?><a.elem xmi.version='2.0'/>", textfile);
            fclose(textfile);

			TiXmlDocument doc;
            doc.LoadFile( "test5.xml" );
            XmlTest( "dot in element attributes and names", doc.Error(), 0);
		}
    }

	{
		FILE* textfile = fopen( "test6.xml", "w" );
		if ( textfile )
		{
            fputs("<element><Name>1.1 Start easy ignore fin thickness&#xA;</Name></element>", textfile );
            fclose(textfile);

            TiXmlDocument doc;
            bool result = doc.LoadFile( "test6.xml" );
            XmlTest( "Entity with one digit.", result, true );

			TiXmlText* text = doc.FirstChildElement()->FirstChildElement()->FirstChild()->ToText();
			XmlTest( "Entity with one digit.",
						text->Value(), "1.1 Start easy ignore fin thickness\n" );
		}
    }


	#if defined( WIN32 ) && defined( TUNE )
	QueryPerformanceCounter( (LARGE_INTEGER*) (&end) );
	QueryPerformanceFrequency( (LARGE_INTEGER*) (&freq) );
	printf( "Time for run: %f\n", ( double )( end-start ) / (double) freq );
	#endif

	printf ("\nPass %d, Fail %d\n", gPass, gFail);
	return gFail;
}

