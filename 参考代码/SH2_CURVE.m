function sh = SH2_CURVE(t,x4)
    % Function calculates time delay for HU2
    if t <= 0.0
       sh=0.0;
    elseif t <= x4
       sh=0.5*(t/x4)^(2.5);
    elseif t < 2*x4
       sh=1-0.5*(2-t/x4)^(2.5);
    elseif t >= 2*x4
       sh=1.0;
    end
end