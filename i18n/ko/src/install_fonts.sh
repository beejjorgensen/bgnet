mkdir -p /usr/share/fonts/truetype/pretendard
cp ../fonts/pretendard/public/variable/PretendardVariable.ttf /usr/share/fonts/truetype/pretendard/PretendardVariable.ttf
cp ../fonts/pretendard/public/static/alternative/* /usr/share/fonts/truetype/pretendard/
fc-cache -f -v
fc-list
