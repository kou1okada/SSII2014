CXXFLAGS+=-mavx
CXXFLAGS+=-fno-exceptions
LDLIBS+=-lopencv_core -lopencv_highgui -lopencv_imgproc

SSII2014: main.o bilateralSample.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
clean:
	-rm SSII2014 main.o bilateralSample.o
