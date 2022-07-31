# Face Detection Service


### Build proto

```
/usr/bin/protoc -I/usr/include -I. \
	 --cpp_out=./api \
	 --grpc_out=./api \
	 --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin \
	 ./protos/server.proto
```
