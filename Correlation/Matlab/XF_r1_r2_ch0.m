clear; clc; close all;

% X - F means X correlate first
% F freq domain after correlation

% compute mean of angles
mangle = @(x) angle(mean(exp(1j*angle(x))));
% remove mean angle
rmangle=@(x)  abs(x).*exp(1j*angle(x.*exp(-1j*mangle(x))));

do_true_correlCoeff = 1; % 0 to show unnormalized cross-corr, 1 to show correlation coefficient

nlags  = 20000;
upfact = 20;
vec_avg = 100;

Fs1   = 125e6;
Fs2   = 4096e6;

% nframes related to nlags

% total data read in relates to SNR for fringe search, need more
% than nlags which is only searching through 1 and 4 frames, r1 r2 respec.
% could skip by frames equal to nlags though, to find close up data

n1frames = 20;
n2frames = 40;

Nch1 = 16; % number of channels in file 1 (file 2 assumed to have exactly 1 channel)

skip_r1_frames = 0;
skip_r2_frames = 14+floor(125*9.1);


% read in r1 data
if 1,
    % file_r1 = 'data/dbbc_t1_short.vdif';
    file_r1 = 'data/Db_DBBC62.5xR2DBE2048.vdif';
    r1_frames = read_n_dbbc2_frames(file_r1,n1frames,skip_r1_frames);
    dr1       = get_data_from_frames(r1_frames);
    Nch1 = 16;
else
    file_r1 = 'data/r2dbe_t1_short.vdif';
    r1_frames = read_n_r2_frames(file_r1,n2frames,skip_r2_frames);
    dr1       = get_data_from_frames(r1_frames);
    Nch1 = 1;
end

% read in r2 data
% file_r2 = 'data/r2dbe_t0_short.vdif';
file_r2 = 'data/Ra_DBBC62.5xR2DBE2048.vdif';
r2_frames = read_n_r2_frames(file_r2,n2frames,skip_r2_frames);
dr2       = get_data_from_frames(r2_frames);

h=figure(1);
hold on
col = 0.9*hsv(Nch1);

init_dly_us = find_initial_offset(r1_frames{1}.datetime,r2_frames{1}.datetime);

dlys = [];
dr1_adj = {};
dr2_adj = {};
for chn = 1:Nch1
        Fs_init = r1_frames{1}.Fs;
        Fs_final = Fs_init/2;
        Fs_shift = Fs_init/4;
        is_flipped = r1_frames{1}.is_flipped;
        flo = r1_frames{1}.ch{chn}.flo;
        fhi = r1_frames{1}.ch{chn}.fhi;

        d1final = prep_chan(Fs_init,Fs_final,Fs_shift,is_flipped,dr1{chn});
        dr1_adj{chn}.data=d1final;


        % take r2 data and match selected channel
        Fs_init = r2_frames{1}.Fs;
        % use same Fs_final
        Fs_shift  = mean([flo,fhi])*1e6;
        is_flipped = r2_frames{1}.is_flipped;


        % decimate by factor of

        d2final = prep_chan(Fs_init,Fs_final,Fs_shift,is_flipped,dr2{1});
        dr2_adj{chn}.data=d2final;

        Fs = Fs_final;
        dr1_adj{chn}.Fs = Fs;
        dr2_adj{chn}.Fs = Fs;

        dr1_adj{chn}.flo = flo;
        dr1_adj{chn}.fhi = fhi;
        dr2_adj{chn}.flo = flo;
        dr2_adj{chn}.fhi = fhi;

end



% correlate data
col = [col; col(1,:)];
% for chn = 1:17
for chn = 1:Nch1

%     end
    Fs = dr1_adj{chn}.Fs;
    flo = dr1_adj{chn}.flo;
    fhi = dr1_adj{chn}.fhi;
    Ldata = length(dr1_adj{chn}.data);

    [x1,x2adj,loc_dly_samp] = find_dly(dr1_adj{chn}.data,dr2_adj{chn}.data,upfact,nlags);
    tot_dly_samp = init_dly_us*Fs/1e6 + loc_dly_samp;
    tot_dly_us = -loc_dly_samp/Fs*1e6 + init_dly_us;

    [c,lags] = xcorr(x1,x2adj,nlags);
    x = fftshift(c);
    X = fftshift(fft(x));
    clear x;

    if do_true_correlCoeff,
        [ac1,~] = xcorr(x1,x1,nlags);
        [ac2,~] = xcorr(x2adj,x2adj,nlags);
        y1 = ifftshift(ac1);
        y2 = ifftshift(ac2);
        clear ac1 ac2;
        Y1 = fftshift(fft(y1));
        Y2 = fftshift(fft(y2));
        clear y1 y2;
        X = X ./ sqrt(Y1.*Y2);
    end

    Y = vec_sum(X,vec_avg);
    Y = rmangle(Y);
    L=length(Y);
    freq = linspace(flo,fhi,L);

    subplot(2,1,1)
    hold on
    if ~do_true_correlCoeff,
        plot(freq,(10*log10(abs(Y))),'color',col(chn,:))
        txty = 0;
    else
        plot(freq,abs(Y),'color',col(chn,:))
        txty = 0.1;
    end
    txtx = (r1_frames{1}.ch{chn}.fhi + r1_frames{1}.ch{chn}.flo) / 2;
    text(txtx,txty,['Ch ',num2str(chn-1)])

    subplot(2,1,2)
    hold on
    plot(freq,(angle(Y)),'x','color',col(chn,:))
    chinfos = { ...
            ['{\Delta}n=' num2str(loc_dly_samp,'%.1f')], ...
            [num2str(tot_dly_us,'%.2f'),' {\mu}s'] ...
    };
    text(txtx,pi*0.75, chinfos, 'HorizontalAlignment','center')

    dlys = [dlys tot_dly_us];
end

Xlab = {};
k=1;
fval = 0:Fs1/2/1e6:1024;
for l=1:length(fval)
    Xlab{k} = num2str(fval(l));
    k=k+1;
end

len_s = Ldata/Fs;
len_ms = len_s*1000;

subplot(2,1,1)
if ~do_true_correlCoeff,
    title({['Xcorr in Freq Domain: ',num2str(len_ms),' ms int time']})
else
    title({['Normalized Xcorr in Freq Domain: ',num2str(len_ms),' ms int time']})
end
xlim([r1_frames{1}.ch{1}.flo fval(end)])
xlabel('Freq (MHz)')
set(gca,'XTick',fval)
set(gca,'XTickLabel',Xlab)

if ~do_true_correlCoeff,
    ylabel('Mag (dB)')
    %ylim([0 60])
else
    ylabel('Correl. coefficient')
    %ylim([0 60])
end
grid off

subplot(2,1,2)
med_dly = median(dlys);
if med_dly<0;
    str = 'DBBC time is ahead of R2DBE (same data in DBBC has later time stamp)';
else
    str = 'R2DBE time is ahead of DBBC (same data in R2DBE has later time stamp)';
end
title(['Median shows ',str,' by ',num2str(med_dly),' us, Vec avg = ',num2str(vec_avg),' samples'])

xlabel('Freq (MHz)')
xlim([r1_frames{1}.ch{1}.flo fval(end)])
set(gca,'XTick',fval)
set(gca,'XTickLabel',Xlab)
ylabel('Phase (rad)')
ylim([-pi pi])
set(gca,'YTick',[-pi:pi/2:pi])
set(gca,'YTickLabel',{'-pi','-pi/2','0','pi/2','pi'});

grid off
