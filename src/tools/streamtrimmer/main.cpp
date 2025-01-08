
#include <cstdlib>
#include <stdio.h>


#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>

#include <wondergfx.h>
#include <wondergfxstream.h>


using namespace wg;
using namespace std;


//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{
	if( argc < 2 )
	{
		cerr << string("ERROR: Input file not specified\n");
		return 1;
	}

	std::ifstream input( argv[1], ifstream::binary );
	if( !input )
	{
		cerr << "ERROR: File not found\n";
		return 2;
	}
	
	input.seekg(0, input.end);
	int length = input.tellg();
	input.seekg(0, input.beg);
		
	std::ofstream trimmed( string("trimmed_") + argv[1], fstream::binary );
	std::ofstream log( string("log_") + argv[1], fstream::binary );
	std::ofstream log_trimmed( string("log_trimmed_") + argv[1], fstream::binary );

	
	GfxBase::init();
	
	auto pStreamReader = StreamReader::create( [&](int amount, void * pDest){
		int toRead = min(length,amount);
		input.read((char *)pDest, toRead);
		length -= toRead;
		return toRead;
	} );

	
	auto pTrimmedLogger = StreamLogger::create(log_trimmed);
	
	auto pTrimmedWriter = StreamWriter::create( [&](int amount, const void * pDest){
		trimmed.write((const char*)pDest, amount);
	} );

	auto pTrimmedSplitter = StreamSplitter::create( {{pTrimmedLogger, pTrimmedLogger->input}, {pTrimmedWriter, pTrimmedWriter->input}});
	
	auto pTrimEncoder = StreamEncoder::create( {pTrimmedSplitter, pTrimmedSplitter->input} );
	
	auto pSurfaceFactory = StreamSurfaceFactory::create(pTrimEncoder);
	auto pEdgemapFactory = StreamEdgemapFactory::create(pTrimEncoder);

	auto pBackend = StreamBackend::create(pTrimEncoder);
	
	
	auto pPlayer = StreamPlayer::create(pBackend, pSurfaceFactory, pEdgemapFactory);
	
	auto pStraightLogger = StreamLogger::create( log );

	auto pInputSplitter = StreamSplitter::create( {{pStraightLogger, pStraightLogger->input}, {pPlayer, pPlayer->input}} );
//	auto pInputSplitter = StreamSplitter::create( {{pPlayer, pPlayer->input}} );

	auto pStreamPump = StreamPump::create( {pStreamReader, pStreamReader->output}, {pInputSplitter, pInputSplitter->input} );
	
	
	pBackend->defineCanvas(CanvasRef::Canvas_1, {240,240}, PixelFormat::Undefined, 64);
	pBackend->defineCanvas(CanvasRef::Canvas_2, {240,240}, PixelFormat::Undefined, 64);

	pStraightLogger->setDisplayOffset(false);
	pStraightLogger->setDisplayChunkNb(false);
	pTrimmedLogger->setDisplayOffset(false);
	pTrimmedLogger->setDisplayChunkNb(false);

	
	
	pStreamPump->pumpAll();
	
	input.close();
	trimmed.close();
	log.close();
	log_trimmed.close();
	
    return 0;
}




