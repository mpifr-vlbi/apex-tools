function m5specGlueplot(fn)

 dd = dlmread(fn);
 dd = dd(2:end,:); % remove 0Hz
 Nch = size(dd,1);
 Nif = size(dd,2) - 1;
 
 f = dd(:,1);
 fmax = max(f);
 amax = 0;
 amin = 1e32;
 
 figure(1),clf;
 cm = jet(Nif);
 for ii=1:Nif,
     semilogy(f + (ii-1)*fmax, dd(:,ii+1), '-', 'color', cm(ii,:));
     amax = max([amax, max(dd(:,ii+1))]);
     amin = max([0.1 min([amin, min(dd(:,ii+1))])]);
     hold on;
 end
 aa = [0 Nif*fmax amin amax];
 axis(aa);
 xlim([0 Nif*fmax]);
 ylim([amin amax]);
 
 if 0,
   aa(3)=0.24;
   aa(4)=10^2;
   axis(aa);
 end
 for ii=2:Nif,
     plot([(ii-1)*fmax, (ii-1)*fmax], [amin amax], 'k:');
 end

 for ii=1:Nif,
    x = (ii-1)*fmax + 0.1*fmax;
    y = aa(4)*0.85;
    text(x,y, num2str(ii-1,'%02d'));
 end
 xlabel('Frequency (MHz)');
 ylabel('Log Amplitude');
 
 [pathstr,name,ext] = fileparts(fn);
 tstr = strrep(name, '_', '\_');  % required since title() uses LaTeX parser
 title(tstr);
 
 %print -dpsc 'm5spec.ps';
 f2 = sprintf('%s.ps', name);
 print(gcf(), '-dpsc', f2);
