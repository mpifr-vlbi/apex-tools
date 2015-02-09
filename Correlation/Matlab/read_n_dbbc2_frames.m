function [frames,d] = read_n_dbbc2_frames(file,n1frames,skip_frames)

    pkt_B = 8032;

    bytes_skip = skip_frames*pkt_B;

    fid = fopen(file,'r');
    fseek(fid,bytes_skip,'bof');

    for f = 1:n1frames
        [frame,fid] = read_dbbc2_frame(fid,pkt_B);
        frames{f} = frame;
    end
    fclose(fid);

end

function [frame,fid] = read_dbbc2_frame(fid,len_Bytes)

    % DBBC 62500 frames per sec in single 16x62.5MHz=1000 MHz IF
    fs = 2*1000e6; nbit = 2; pkt_payload = 8000;
    fps = fs*nbit / (8*pkt_payload);

    frame = struct();

    [frame,fid,count]=read_vdif_hdr_dbbc(fid,frame);

    hdr_len = count; % 8 32 bit words
    data_len = (len_Bytes/4) - hdr_len;

    frame.datetime = get_date_from_vdif(frame.secs_since_epoch,frame.ref_epoch,frame.data_frame,fps);
    frame.Fs = 62.5e6*2;
    frame.len_us = 1/fps*1e6;
    frame.frames_per_sec = fps;
    frame.is_flipped = 0;
    frame.samp_per_frame = 2500;


    [frame,fid] = read_vdif_data_dbbc(fid,frame,data_len);
end

function [frame,fid,count] = read_vdif_hdr_dbbc(fid,frame)

    [words,count1]=fread(fid,4,'uint32', 0, 'ieee-le');

    % word 0
    frame.invalid_data     = boolean(bitand(bitshift(words(1),-31),hex2dec('1')));
    frame.legacy_mode      = boolean(bitand(bitshift(words(1),-30),hex2dec('1')));
    frame.secs_since_epoch = bitand(words(1),hex2dec('3fffffff'));

    % word 1
    frame.ref_epoch  = bitand(bitshift(words(2),-24),hex2dec('3f'));
    frame.data_frame = bitand(words(2),hex2dec('ffffff'));

    % word 2
    frame.vdif_vers    = bitand(bitshift(words(3),-29),hex2dec('7'));
    frame.log2_chans   = bitand(bitshift(words(3),-24),hex2dec('1f'));
    frame.frame_length = bitand(words(3),hex2dec('ffffff'));

    % word 3
    frame.complex         = boolean(bitand(bitshift(words(4),-31),hex2dec('1')));
    frame.bits_per_sample = 1 + bitand(bitshift(words(4),-26),hex2dec('1f'));
    frame.thread_id       = bitand(bitshift(words(4),-16),hex2dec('3ff'));
    frame.station_id      = bitand(words(4),hex2dec('ffff'));

    % parse extended user data

    count2 = 0;
    % words 4-7
    if ~(frame.legacy_mode) % if not leg mode, more data to read
        [words(5:8),count2] = fread(fid,4,'uint32', 0, 'ieee-le');
        frame.mk5b_w0  = words(5);
        frame.mk5b_w1  = words(6);
        frame.mk5b_w2  = words(7);
        frame.mk5b_w3  = words(8);
    end

    count = count1+count2;

end

function [frame,fid] = read_vdif_data_dbbc(fid,frame,data_len)

    tmp = fread(fid,data_len,'uint32', 0, 'ieee-le');

    for ch = 1:16
        Fs_MHz = frame.Fs/2/1e6;
        if frame.is_flipped
            chlo = Fs_MHz/2 +Fs_MHz*(Fs_MHz/2-ch); %range in MHz
            chhi = Fs_MHz/2 +Fs_MHz*(Fs_MHz/2-ch+1);
        else
            chlo = -Fs_MHz/2 +Fs_MHz*(ch-1); %range in MHz
            chhi = -Fs_MHz/2 +Fs_MHz*(ch);
        end
       dat  =bitand(bitshift(tmp,-2*(ch-1)), hex2dec('3'));

       vals = [-3 1 -1 3] ;
       % JanW: seems wrong encoding for VDIF?  should be --,-,+,++ e.g. [-7, -2, +2, +7] ?
       vals = [-3.3359,-1.0,1.0,3.3359];

       dat=vals(dat+1);
       % ch, chlo, chhi
       frame.ch{ch}.flo = chlo;
       frame.ch{ch}.fhi = chhi;
       frame.ch{ch}.data = dat;
    end
end
