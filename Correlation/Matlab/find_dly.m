function [x1,x2adj,dly_samp] = find_dly(x1,x2,upfact,nlags)


% cross correlate, find lag
    [c_orig,lags] = xcorr(x1,x2,nlags);
    
    % find and apply coarse delay
    ind = find( abs(c_orig)==max(abs(c_orig)) );
    x2shift = circshift(x2,[0 lags(ind)]);
    
    % redo xcorr
    [c_crs,lags] = xcorr(x1,x2shift,nlags);
    
    
    coarse_dly = lags(ind);

    
    % find and apply fine delay
    x1up = interp(x1,     upfact);
    x2up = interp(x2shift,upfact);
    
    [c_inter,lags]=xcorr(x1up,x2up,upfact);  %it already aligns at 0, just go out to lag upfact
    
    
    % find, apply
    ind_inter = find(abs(c_inter)==max(abs(c_inter)));
    x2upshift = circshift(x2up,[0 lags(ind_inter)]);
    
    dly_samp    = lags(ind_inter)/10 + coarse_dly;
    
    x2adj       = decimate(x2upshift,upfact);