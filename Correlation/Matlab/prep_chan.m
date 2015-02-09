function d1final = prep_chan(Fs_init,Fs_final,Fs_shift,is_flipped,data)
    % take ch data and shift spectrum
    dfact = Fs_init/Fs_final;
    d1 = data;  % from 528-560, mk complex
    n = 0:length(d1)-1;
    
    d1cplx = d1.*exp((-1)^(is_flipped+1)*1j*2*pi*Fs_shift/Fs_init*n);
    % decimate to remove extra spectrum
    d1final = resample(d1cplx,Fs_final,Fs_init);