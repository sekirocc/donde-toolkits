FROM ncnn:20211208 as builder

ENV LD_LIBRARY_PATH=/face_service/deps/ncnn-20211208-ubuntu-2004-shared/lib

WORKDIR /face_service

COPY . ./

FROM ncnn:20211208

WORKDIR /face_service

COPY --from=builder /face_service/build/bin/ImageProcess /face_service/ImageProcess
COPY --from=builder /face_service/deps /face_service/deps
COPY --from=builder /face_service/examples /face_service/examples
COPY --from=builder /face_service/models /face_service/models
ENV LD_LIBRARY_PATH=/face_service/deps/ncnn-20211208-ubuntu-2004-shared/lib

CMD ["/face_service/ImageProcess", "--config_path", "/face_service/examples/server.json", "--loglevel", "error"]
